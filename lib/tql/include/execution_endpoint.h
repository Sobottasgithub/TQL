#ifndef EXECUTION_ENDPOINT_H
#define EXECUTION_ENDPOINT_H

#include <tablog.h>
#include <memory>
#include <arrow/table.h>

namespace tql {
  class ExecutionEndpoint {
    public:
      ExecutionEndpoint();

      std::shared_ptr<arrow::Table> openFile(std::string filePath);

    private:
      std::shared_ptr<tablog::Tablog> logger;
  };
}

#endif
