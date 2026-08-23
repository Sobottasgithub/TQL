#include "../include/execution_endpoint.h"

#include <arrow/api.h>
#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/acero/util.h>
#include <arrow/compute/api.h>
#include <arrow/compute/expression.h>
#include <arrow/compute/initialize.h>
#include <arrow/csv/api.h>
#include <arrow/csv/options.h>
#include <arrow/io/api.h>
#include <arrow/scalar.h>
#include <arrow/table.h>
#include <arrow/type.h>

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <tablog.h>
#include <utility>

namespace tql {
  namespace {
    std::runtime_error arrowError(std::string context, const arrow::Status& status) {
      return std::runtime_error(context + ": " + status.ToString());
    }

    template <typename T>
    T unwrapOrThrow(arrow::Result<T> result, std::string context) {
      if (!result.ok()) {
        throw arrowError(std::move(context), result.status());
      }

      return std::move(result).ValueOrDie();
    }

    void requireTable(const std::shared_ptr<arrow::Table>& table, const std::string& operationName) {
      if (!table) {
        throw std::invalid_argument(operationName + " requires a table");
      }
    }
  }

  ExecutionEndpoint::ExecutionEndpoint() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("ExecutionEndpoint", true);
    this->logger = logger;

    arrow::Status status = arrow::compute::Initialize();
    if (!status.ok()) {
      throw arrowError("Unable to initialize Arrow compute", status);
    }
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::openFile(std::string filePath) {
    std::filesystem::path path = std::filesystem::absolute(filePath).lexically_normal();

    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
      throw std::invalid_argument("Invalid filepath: " + filePath);
    }

    const std::string cacheKey = path.string();
    const std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(path);
    const std::uintmax_t fileSize = std::filesystem::file_size(path);

    auto cachedTable = this->tableCache.find(cacheKey);
    if (cachedTable != this->tableCache.end()
        && cachedTable->second.lastWriteTime == lastWriteTime
        && cachedTable->second.fileSize == fileSize) {
      return cachedTable->second.table;
    }

    arrow::io::IOContext ioContext = arrow::io::default_io_context();
    std::shared_ptr<arrow::io::ReadableFile> file = unwrapOrThrow(
        arrow::io::ReadableFile::Open(cacheKey),
        "Unable to open CSV file");
    std::shared_ptr<arrow::io::InputStream> fileInput = file;

    arrow::csv::ReadOptions readOptions = arrow::csv::ReadOptions::Defaults();
    arrow::csv::ParseOptions parseOptions = arrow::csv::ParseOptions::Defaults();
    arrow::csv::ConvertOptions convertOptions = arrow::csv::ConvertOptions::Defaults();

    readOptions.use_threads = true;

    std::shared_ptr<arrow::csv::TableReader> reader = unwrapOrThrow(
        arrow::csv::TableReader::Make(ioContext,
                                      fileInput,
                                      readOptions,
                                      parseOptions,
                                      convertOptions),
        "Unable to instantiate CSV TableReader");

    std::shared_ptr<arrow::Table> table = unwrapOrThrow(
        reader->Read(),
        "Unable to read CSV table");

    this->tableCache[cacheKey] = CachedTable{lastWriteTime, fileSize, table};
    return table;
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::getWhere(std::string operatorName, std::string columnName, std::string compareValue, std::shared_ptr<arrow::Table> table) {
    requireTable(table, "WHERE");

    int col_idx = table->schema()->GetFieldIndex(columnName);
    if (col_idx == -1) {
      throw std::invalid_argument("Unknown or ambiguous WHERE column: " + columnName);
    }

    auto target_type = table->schema()->field(col_idx)->type();
    auto string_scalar = std::make_shared<arrow::StringScalar>(compareValue);
    arrow::Datum cast_result = unwrapOrThrow(
        arrow::compute::Cast(arrow::Datum(string_scalar), target_type),
        "Unable to cast WHERE value '" + compareValue + "' to " + target_type->ToString());

    std::shared_ptr<arrow::Scalar> converted_scalar = cast_result.scalar();
    arrow::compute::Expression field = arrow::compute::field_ref(columnName);
    arrow::compute::Expression literal = arrow::compute::literal(converted_scalar);

    if (operatorName != "=") {
      throw std::invalid_argument("Unsupported WHERE operator: " + operatorName);
    }

    arrow::compute::Expression filter_expr = arrow::compute::equal(field, literal);

    arrow::acero::Declaration declaration = arrow::acero::Declaration::Sequence({
        {"table_source", arrow::acero::TableSourceNodeOptions(table)},
        {"filter", arrow::acero::FilterNodeOptions(filter_expr)}
    });

    return unwrapOrThrow(
        arrow::acero::DeclarationToTable(declaration, /*use_threads=*/true),
        "Unable to execute WHERE filter");
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::selectColumns(std::vector<std::string> columnNames, std::shared_ptr<arrow::Table> table) {
    requireTable(table, "SELECT");

    std::vector<int> columnIndices;
    columnIndices.reserve(columnNames.size());

    for (int index = 0; index < columnNames.size(); ++index) {
      int columnIndex = table->schema()->GetFieldIndex(columnNames.at(index));

      if (columnIndex == -1) {
        throw std::invalid_argument("Unknown or ambiguous SELECT column: " + columnNames.at(index));
      }

      columnIndices.push_back(columnIndex);
    }
    
    return unwrapOrThrow(
        table->SelectColumns(columnIndices),
        "Unable to select columns");
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::getDistinct(std::shared_ptr<arrow::Table> table) {    
    requireTable(table, "DISTINCT");

    arrow::acero::Declaration source{
        "table_source", 
        arrow::acero::TableSourceNodeOptions(table)
    };

    std::vector<std::string> distinctColumns = table->ColumnNames();
    std::vector<arrow::FieldRef> keys;
    for (int index = 0; index < distinctColumns.size(); ++index) {
      keys.push_back(arrow::FieldRef(distinctColumns.at(index)));
    }

    arrow::acero::Declaration aggregate {
        "aggregate",
        arrow::acero::AggregateNodeOptions{/*aggregates=*/{}, /*keys=*/keys}
    };

    arrow::acero::Declaration plan = arrow::acero::Declaration::Sequence({source, aggregate});

    return unwrapOrThrow(
        arrow::acero::DeclarationToTable(plan, /*use_threads=*/true),
        "Unable to execute DISTINCT");
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::getCount(std::shared_ptr<arrow::Table> table) {
    requireTable(table, "COUNT");

    return makeSingleColumnSingleRowTable(
        "count",
        std::make_shared<arrow::Int64Scalar>(table->num_rows()));
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::getMin(std::shared_ptr<arrow::Table> table) {
    return aggregateMinMax("min", table, 0);
  }
  
  std::shared_ptr<arrow::Table> ExecutionEndpoint::getMax(std::shared_ptr<arrow::Table> table) {
    return aggregateMinMax("max", table, 1);
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::getRenamedTable(std::string originalColumnName, std::string newColumnName, std::shared_ptr<arrow::Table> table) {    
    requireTable(table, "AS");

    int columnIndex = table->schema()->GetFieldIndex(originalColumnName);
    if (columnIndex == -1) {
      throw std::invalid_argument("Unknown or ambiguous column for AS: " + originalColumnName);
    }

    std::vector<std::string> columnNames = table->ColumnNames();
    columnNames[columnIndex] = newColumnName;

    return unwrapOrThrow(
        table->RenameColumns(columnNames),
        "Unable to rename column");
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::aggregateMinMax(std::string fieldName, std::shared_ptr<arrow::Table> table, int resultIndex) {
    requireTable(table, fieldName);

    if (table->num_columns() != 1) {
      throw std::invalid_argument(fieldName + " requires exactly one input column");
    }

    arrow::compute::ScalarAggregateOptions options;
    options.skip_nulls = true;
    options.min_count = 1;

    arrow::Datum result = unwrapOrThrow(
        arrow::compute::MinMax(arrow::Datum(table->column(0)), options),
        "Unable to calculate " + fieldName);

    const auto& minMaxScalar = result.scalar_as<arrow::StructScalar>();
    if (resultIndex < 0 || resultIndex >= minMaxScalar.value.size()) {
      throw std::runtime_error("Arrow MinMax returned an unexpected result shape");
    }

    return makeSingleColumnSingleRowTable(fieldName, minMaxScalar.value.at(resultIndex));
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::makeSingleColumnSingleRowTable(std::string fieldName, std::shared_ptr<arrow::Scalar> value) {
    if (!value) {
      throw std::invalid_argument("Cannot build result table from a null scalar pointer");
    }

    std::shared_ptr<arrow::Array> array = unwrapOrThrow(
        arrow::MakeArrayFromScalar(*value, 1),
        "Unable to build scalar result array");
    std::shared_ptr<arrow::Field> field = arrow::field(fieldName, value->type);
    std::shared_ptr<arrow::Schema> schema = arrow::schema({field});
    std::shared_ptr<arrow::Table> resultTable = arrow::Table::Make(schema, {array});

    return resultTable;
  }
}
