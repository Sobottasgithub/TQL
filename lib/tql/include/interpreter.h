#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"

#include <tablog.h>
#include <memory>
#include <optional>
#include <functional>
#include <arrow/table.h>
#include <string>

namespace tql {
  class Interpreter {
    public:
      using OpenFile = std::function<std::shared_ptr<arrow::Table>(std::string)>;
      using GetWhere = std::function<std::shared_ptr<arrow::Table>(std::string, std::string, std::string, std::shared_ptr<arrow::Table>)>;
      using SelectColumns = std::function<std::shared_ptr<arrow::Table>(std::vector<std::string>, std::shared_ptr<arrow::Table>)>;
      using GetDistinct = std::function<std::shared_ptr<arrow::Table>(std::shared_ptr<arrow::Table>)>;
      using GetCount = std::function<std::shared_ptr<arrow::Table>(std::shared_ptr<arrow::Table>)>;
      using GetMax = std::function<std::shared_ptr<arrow::Table>(std::shared_ptr<arrow::Table>)>;
      using GetMin = std::function<std::shared_ptr<arrow::Table>(std::shared_ptr<arrow::Table>)>;
      using GetSum = std::function<std::shared_ptr<arrow::Table>(std::shared_ptr<arrow::Table>)>;
      using GetAvg = std::function<std::shared_ptr<arrow::Table>(std::shared_ptr<arrow::Table>)>;
      using GetRenamedTable = std::function<std::shared_ptr<arrow::Table>(std::string, std::string, std::shared_ptr<arrow::Table>)>;
      
      Interpreter();

      void setOpenFile(OpenFile openFile);
      void setGetWhere(GetWhere getWhere);
      void setSelectColumns(SelectColumns selectColumns);
      void setGetDistinct(GetDistinct getDistinct);
      void setGetCount(GetCount getCount);
      void setGetMin(GetMin getMin);
      void setGetMax(GetMax getMax);
      void setGetSum(GetSum getSum);
      void setGetAvg(GetAvg getAvg);
      void setGetRenamedTable(GetRenamedTable getRenamedTable);
      
      std::shared_ptr<arrow::Table> interpret(Parser::Expression expressionTree);
      
    private:
      std::shared_ptr<tablog::Tablog> logger;

      OpenFile openFile;
      GetWhere getWhere;
      SelectColumns selectColumns;
      GetDistinct getDistinct;
      GetCount getCount;
      GetMin getMin;
      GetMax getMax;
      GetAvg getAvg;
      GetSum getSum;
      GetRenamedTable getRenamedTable;

      std::shared_ptr<arrow::Table> interpretSelect(Parser::Expression expression);
      std::shared_ptr<arrow::Table> interpretFrom(Parser::Expression expression);
      std::shared_ptr<arrow::Table> interpretWhere(Parser::Expression expression, std::shared_ptr<arrow::Table> table);
      std::shared_ptr<arrow::Table> interpretColumns(Parser::Expression expression, std::shared_ptr<arrow::Table> table);
      std::shared_ptr<arrow::Table> interpretAtom(Parser::Expression expression, std::shared_ptr<arrow::Table> table);
      std::shared_ptr<arrow::Table> interpretDistinct(std::shared_ptr<arrow::Table>);
      std::shared_ptr<arrow::Table> interpretCount(Parser::Expression expression, std::shared_ptr<arrow::Table> table);
      std::shared_ptr<arrow::Table> interpretAs(std::string originalColumnName, Parser::Expression expression, std::shared_ptr<arrow::Table> table);

      std::shared_ptr<arrow::Table> interpretAggregate(Parser::Expression expression, std::shared_ptr<arrow::Table> table);
      std::optional<Parser::Expression> getExpressionByTokenType(TokenType requestedTokenType, std::vector<Parser::Expression> expressions);
  };
}

#endif
