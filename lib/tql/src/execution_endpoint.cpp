#include "../include/execution_endpoint.h"

#include <stdexcept>
#include <tablog.h>
#include <filesystem>
#include <memory>
#include <arrow/table.h>
#include <arrow/csv/api.h>
#include <arrow/io/api.h>
#include <arrow/api.h>
#include <arrow/csv/options.h>
#include <arrow/csv/api.h>
#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/compute/api.h>
#include <arrow/compute/expression.h>
#include <arrow/compute/initialize.h>
#include <arrow/dataset/api.h>
#include <arrow/acero/util.h>
#include <arrow/acero/exec_plan.h>
#include <arrow/acero/util.h>
#include <arrow/chunked_array.h>
#include <arrow/scalar.h>
#include <arrow/type.h>

namespace tql {
  ExecutionEndpoint::ExecutionEndpoint() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("ExecutionEndpoint", true);
    this->logger = logger;

    arrow::Status status = arrow::compute::Initialize();
    if (!status.ok()) {
      throw "Unable to initialize Arrow compute";
    }
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::openFile(std::string filePath) {
    if (std::filesystem::exists(filePath)) {
      arrow::io::IOContext ioContext = arrow::io::default_io_context();

      arrow::Result<std::shared_ptr<arrow::io::ReadableFile>> maybeFile = arrow::io::ReadableFile::Open(filePath);
      std::shared_ptr<arrow::io::InputStream> fileInput = *maybeFile;

      arrow::csv::ReadOptions readOptions = arrow::csv::ReadOptions::Defaults();
      arrow::csv::ParseOptions parseOptions = arrow::csv::ParseOptions::Defaults();
      arrow::csv::ConvertOptions convertOptions = arrow::csv::ConvertOptions::Defaults();

      arrow::Result<std::shared_ptr<arrow::csv::TableReader>> maybeReader = arrow::csv::TableReader::Make(ioContext,
                                                      fileInput,
                                                      readOptions,
                                                      parseOptions,
                                                      convertOptions);
      if (!maybeReader.ok()) {
        this->logger->log(tablog::CRITICAL, "Error while instantiating TableReader!");
        throw "Error while instantiating TableReader!";
      }
      std::shared_ptr<arrow::csv::TableReader> reader = *maybeReader;

      arrow::Result<std::shared_ptr<arrow::Table>> maybeTable = reader->Read();
      if (!maybeTable.ok()) {
        this->logger->log(tablog::CRITICAL, "Error while read table from CSV file!");
        throw "Error while read table from CSV file!";
      }
      std::shared_ptr<arrow::Table> table = maybeTable.ValueOrDie();
      return std::move(table);
    } else {
      throw std::invalid_argument( "Invalid filepath!" );
      return nullptr;
    }
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::getWhere(std::string operatorName, std::string columnName, std::string compareValue, std::shared_ptr<arrow::Table> table) {
    int col_idx = table->schema()->GetFieldIndex(columnName);
    if (col_idx == -1) {
      throw std::invalid_argument("Unknown or ambiguous WHERE column: " + columnName);
    }

    auto target_type = table->schema()->field(col_idx)->type();
    auto string_scalar = std::make_shared<arrow::StringScalar>(compareValue);
    arrow::Datum cast_result = arrow::compute::Cast(arrow::Datum(string_scalar), target_type).ValueOrDie();

    std::shared_ptr<arrow::Scalar> converted_scalar = cast_result.scalar();
    arrow::compute::Expression field = arrow::compute::field_ref(columnName);
    arrow::compute::Expression literal = arrow::compute::literal(converted_scalar);

    arrow::compute::Expression filter_expr;
    if (operatorName == "<") {
      arrow::compute::Expression filter_expr = arrow::compute::less(field, literal);
      arrow::acero::Declaration declaration = arrow::acero::Declaration::Sequence({
        {"table_source", arrow::acero::TableSourceNodeOptions(table)},
        {"filter", arrow::acero::FilterNodeOptions(filter_expr)}
      });

      return std::move(arrow::acero::DeclarationToTable(declaration, /*use_threads=*/true).ValueOrDie());
    } else if (operatorName == ">") {
      arrow::compute::Expression filter_expr = arrow::compute::greater(field, literal);
      arrow::acero::Declaration declaration = arrow::acero::Declaration::Sequence({
        {"table_source", arrow::acero::TableSourceNodeOptions(table)},
        {"filter", arrow::acero::FilterNodeOptions(filter_expr)}
      });

      return std::move(arrow::acero::DeclarationToTable(declaration, /*use_threads=*/true).ValueOrDie());
    } else if (operatorName == "=") {
      arrow::compute::Expression filter_expr = arrow::compute::equal(field, literal);
      arrow::acero::Declaration declaration = arrow::acero::Declaration::Sequence({
        {"table_source", arrow::acero::TableSourceNodeOptions(table)},
        {"filter", arrow::acero::FilterNodeOptions(filter_expr)}
      });

      return std::move(arrow::acero::DeclarationToTable(declaration, /*use_threads=*/true).ValueOrDie());
    } else {
      throw std::invalid_argument("Unsupported WHERE operator: " + operatorName);
    }
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::selectColumns(std::vector<std::string> columnNames, std::shared_ptr<arrow::Table> table) {
    std::vector<int> columnIndices;

    for (int index = 0; index < columnNames.size(); ++index) {
      int columnIndex = table->schema()->GetFieldIndex(columnNames.at(index));

      if (columnIndex != -1)
        columnIndices.push_back(columnIndex);
    }
    
    arrow::Result<std::shared_ptr<arrow::Table>> resultTable = table->SelectColumns(columnIndices);

    if (resultTable.ok())
      return std::move(resultTable.ValueOrDie());
    return nullptr;
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::getDistinct(std::shared_ptr<arrow::Table> table) {    
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

    return std::move(arrow::acero::DeclarationToTable(plan).ValueOrDie());
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::getCount(std::shared_ptr<arrow::Table> table) {
    return makeSingleColumnSingleRowTable("min", table->num_rows());
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::getMin(std::shared_ptr<arrow::Table> table) {
    const arrow::StructScalar minMaxScalar = getMinMaxAggregate(table);
    std::shared_ptr<arrow::Scalar> minScalar = minMaxScalar.field("min").ValueOrDie();
    int minValue = std::static_pointer_cast<arrow::Int64Scalar>(minScalar)->value;
    
    return makeSingleColumnSingleRowTable("max", minValue);
  }
  
  std::shared_ptr<arrow::Table> ExecutionEndpoint::getMax(std::shared_ptr<arrow::Table> table) {
    const arrow::StructScalar minMaxScalar = getMinMaxAggregate(table);
    std::shared_ptr<arrow::Scalar> maxScalar = minMaxScalar.field("max").ValueOrDie();
    int maxValue = std::static_pointer_cast<arrow::Int64Scalar>(maxScalar)->value;
    
    return makeSingleColumnSingleRowTable("max", maxValue);
  }

  const arrow::StructScalar ExecutionEndpoint::getMinMaxAggregate(std::shared_ptr<arrow::Table> table) {
    if (table->num_columns() != 1) {
      throw std::invalid_argument("Requires exactly one input column");
    }

    arrow::compute::ScalarAggregateOptions options;
    options.skip_nulls = true;
    options.min_count = 1;

    arrow::Datum result = arrow::compute::MinMax(arrow::Datum(table->column(0)), options).ValueOrDie();

    const arrow::StructScalar minMaxScalar = result.scalar_as<arrow::StructScalar>();
    return minMaxScalar;
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::getRenamedTable(std::string originalColumnName, std::string newColumnName, std::shared_ptr<arrow::Table> table) {    
    int columnIndex = table->schema()->GetFieldIndex(originalColumnName);
    std::shared_ptr<arrow::Field> old_field = table->field(columnIndex);
    std::shared_ptr<arrow::Field> new_field = old_field->WithName(newColumnName);

    std::shared_ptr<arrow::ChunkedArray> column_data = table->column(columnIndex);

    return std::move(table->SetColumn(columnIndex, new_field, column_data).ValueOrDie());
  }

  std::shared_ptr<arrow::Int64Scalar> ExecutionEndpoint::getScalarValueFromIndex(std::shared_ptr<arrow::Table> table, int columnIndex, int rowIndex) {
    std::shared_ptr<arrow::Scalar> scalar = table->column(columnIndex)->GetScalar(rowIndex).ValueOrDie();
    std::shared_ptr<arrow::Int64Scalar> intScalar = std::static_pointer_cast<arrow::Int64Scalar>(scalar);
    return intScalar;
  }

  std::shared_ptr<arrow::Table> ExecutionEndpoint::makeSingleColumnSingleRowTable(std::string fieldName, int value) {
    arrow::Int64Builder builder;
    arrow::Status status = builder.Append(value);
    if (!status.ok()) {
      throw "Unable to calc count";
    }
    
    std::shared_ptr<arrow::Array> array;
    status = builder.Finish(&array);
    std::shared_ptr<arrow::Field> field = arrow::field(fieldName, arrow::int64());
    std::shared_ptr<arrow::Schema> schema = arrow::schema({field});
    std::shared_ptr<arrow::Table> resultTable = arrow::Table::Make(schema, {array});

    return std::move(resultTable);
  }
}
