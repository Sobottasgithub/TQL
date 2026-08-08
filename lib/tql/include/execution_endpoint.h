#ifndef EXECUTION_ENDPOINT_H
#define EXECUTION_ENDPOINT_H

#include <tablog.h>
#include <memory>
#include <vector>
#include <arrow/table.h>

namespace tql {
  class ExecutionEndpoint {
    public:
      ExecutionEndpoint();

      std::shared_ptr<arrow::Table> openFile(std::string filePath);
      std::shared_ptr<arrow::Table> selectColumns(std::vector<std::string> columnNames, std::shared_ptr<arrow::Table> table);

    private:
      std::shared_ptr<tablog::Tablog> logger;
  };
}

#endif
