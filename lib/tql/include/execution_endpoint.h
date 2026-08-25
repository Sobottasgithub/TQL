#ifndef EXECUTION_ENDPOINT_H
#define EXECUTION_ENDPOINT_H

#include <tablog.h>
#include <memory>
#include <vector>
#include <arrow/table.h>
#include <arrow/compute/expression.h>

namespace tql {
  class ExecutionEndpoint {
    public:
      ExecutionEndpoint();

      std::shared_ptr<arrow::Table> openFile(std::string filePath);
      std::shared_ptr<arrow::Table> getWhere(std::string operatorName, std::string columnName, std::string compareValue, std::shared_ptr<arrow::Table> table);
      std::shared_ptr<arrow::Table> selectColumns(std::vector<std::string> columnNames, std::shared_ptr<arrow::Table> table);
      std::shared_ptr<arrow::Table> getDistinct(std::shared_ptr<arrow::Table> table);
      std::shared_ptr<arrow::Table> getCount(std::shared_ptr<arrow::Table> table);
      std::shared_ptr<arrow::Table> getMin(std::shared_ptr<arrow::Table> table);
      std::shared_ptr<arrow::Table> getMax(std::shared_ptr<arrow::Table> table);
      const arrow::StructScalar getMinMaxAggregate(std::shared_ptr<arrow::Table> table);
      std::shared_ptr<arrow::Table> getRenamedTable(std::string originalColumnName, std::string newColumnName, std::shared_ptr<arrow::Table> table);

    private:
      std::shared_ptr<tablog::Tablog> logger;

      std::shared_ptr<arrow::Int64Scalar> getScalarValueFromIndex(std::shared_ptr<arrow::Table> table, int columnIndex, int rowIndex);
      std::shared_ptr<arrow::Table> makeSingleColumnSingleRowTable(std::string fieldName, int value);
      std::shared_ptr<arrow::Table> computeWhereFunction(arrow::compute::Expression filterExpression, std::shared_ptr<arrow::Table> table);
  };
}

#endif
