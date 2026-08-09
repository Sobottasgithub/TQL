#include "../include/execution_endpoint.h"

#include <arrow/type.h>
#include <tablog.h>
#include <filesystem>
#include <memory>
#include <arrow/table.h>
#include <arrow/csv/api.h>
#include <arrow/io/api.h>
#include <arrow/api.h>
#include <arrow/csv/options.h>
#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/compute/api.h>
#include <arrow/acero/util.h>
#include <arrow/acero/exec_plan.h>

namespace tql {
  ExecutionEndpoint::ExecutionEndpoint() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("ExecutionEndpoint", true);
    this->logger = logger;
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
    arrow::Int64Builder builder;
    arrow::Status status = builder.Append(table->num_columns());
    if (!status.ok()) {
      throw "Unable to calc count";
    }
    
    std::shared_ptr<arrow::Array> array;
    status = builder.Finish(&array);
    std::shared_ptr<arrow::Field> field = arrow::field("count", arrow::int64());
    std::shared_ptr<arrow::Schema> schema = arrow::schema({field});
    std::shared_ptr<arrow::Table> resultTable = arrow::Table::Make(schema, {array});
    
    return std::move(resultTable);
  }
}
