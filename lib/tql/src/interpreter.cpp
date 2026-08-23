#include "../include/interpreter.h"

#include "../include/token_type.h"

#include <arrow/table.h>
#include <stdexcept>
#include <tablog.h>
#include <memory>
#include <optional>
#include <utility>

/*
* Execution order:
* From
* Where
* Group by
* Having
* Select
* Distinct
* Order by
*/

namespace tql {
  Interpreter::Interpreter() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("Interpreter", true);
    this->logger = logger;
  }

  void Interpreter::setOpenFile(OpenFile openFile) {
    this->openFile = std::move(openFile);
  }

  void Interpreter::setGetWhere(GetWhere getWhere) {
    this->getWhere = std::move(getWhere);
  }

  void Interpreter::setSelectColumns(SelectColumns selectColumns) {
    this->selectColumns = selectColumns;
  }

  void Interpreter::setGetDistinct(GetDistinct getDistinct) {
    this->getDistinct = getDistinct;
  }

  void Interpreter::setGetCount(GetCount getCount) {
    this->getCount = getCount;
  }

  void Interpreter::setGetMin(GetMin getMin) {
    this->getMin = getMin;
  }

  void Interpreter::setGetMax(GetMax getMax) {
    this->getMax = getMax;
  }

  void Interpreter::setGetRenamedTable(GetRenamedTable getRenamedTable) {
    this->getRenamedTable = getRenamedTable;
  }

  std::shared_ptr<arrow::Table> Interpreter::interpret(Parser::Expression expressionTree) {
    if (expressionTree.token.getType() == TokenType::SelectOperator) {
      return interpretSelect(expressionTree);
    } else {
      this->logger->log(tablog::CRITICAL, "Unknown DML operation!");
    }
    return nullptr;
  }

  std::shared_ptr<arrow::Table> Interpreter::interpretSelect(Parser::Expression expression) {
    std::shared_ptr<arrow::Table> resultTable;

    // INFO: Interpret FROM
    std::optional<Parser::Expression> optionalFromExpression = getExpressionByTokenType(TokenType::FromOperator, expression.expressions);
    if (optionalFromExpression.has_value()) {
      Parser::Expression fromExpression = optionalFromExpression.value();
      resultTable = interpretFrom(fromExpression);
    } else {
      throw std::invalid_argument("Missing From expression!");
    }

    // INFO: Interpret WHERE
    std::optional<Parser::Expression> optionalWhereExpression = getExpressionByTokenType(TokenType::WhereOperator, expression.expressions);
    if (optionalWhereExpression.has_value()) {
      Parser::Expression whereExpression = optionalWhereExpression.value();
      resultTable = interpretWhere(whereExpression, resultTable);
    }

    // INFO: Interpret SELECT
    std::optional<Parser::Expression> optionalAllExpression = getExpressionByTokenType(TokenType::All, expression.expressions);
    if (!optionalAllExpression.has_value()) { // The all expression doesnt alter the table
      std::optional<Parser::Expression> optionalColumnsExpression = getExpressionByTokenType(TokenType::Columns, expression.expressions);
      std::optional<Parser::Expression> optionalCountExpression = getExpressionByTokenType(TokenType::CountOperator, expression.expressions);
      std::optional<Parser::Expression> optionalMinExpression = getExpressionByTokenType(TokenType::MinOperator, expression.expressions);
      std::optional<Parser::Expression> optionalMaxExpression = getExpressionByTokenType(TokenType::MaxOperator, expression.expressions);
    
      if (optionalColumnsExpression.has_value()) {
        Parser::Expression columnsExpression = optionalColumnsExpression.value();
        resultTable = interpretColumns(columnsExpression, resultTable);
      } else if (optionalCountExpression.has_value()) {
        Parser::Expression countExpression = optionalCountExpression.value();
        resultTable = interpretCount(countExpression, resultTable);
      } else if (optionalMinExpression.has_value()) {
        Parser::Expression minExpression = optionalMinExpression.value();
        resultTable = interpretMin(minExpression, resultTable);
      } else if (optionalMaxExpression.has_value()) {
        Parser::Expression maxExpression = optionalMaxExpression.value();
        resultTable = interpretMax(maxExpression, resultTable);
      } else {
        this->logger->log(tablog::CRITICAL, "Missing columns or aggregate expression!");
        throw "Missing columns or aggregate expression!";
      }
    }

    // INFO: Interpret DISTINCT
    std::optional<Parser::Expression> optionalDistinctExpression = getExpressionByTokenType(TokenType::DistinctOperator, expression.expressions);
    if (optionalDistinctExpression.has_value()) {
      resultTable = this->getDistinct(resultTable);
    }

    return resultTable;
  }

  std::shared_ptr<arrow::Table> Interpreter::interpretFrom(Parser::Expression expression) {
    Parser::Expression childExpression = expression.expressions.at(0);
    TokenType childTokenType = childExpression.token.getType();
    if (childTokenType == TokenType::Atom) {
      return this->openFile(childExpression.token.getLexeme());
    } else if (childTokenType == TokenType::SelectOperator) {
      return interpretSelect(childExpression);
    } else {
      this->logger->log(tablog::CRITICAL, "Undefined behaviour in from for token type: " + childExpression.token.getTypeAsString());
      throw "Undefined behaviour in from for token type";
    }
    return nullptr;
  }

  std::shared_ptr<arrow::Table> Interpreter::interpretWhere(Parser::Expression expression, std::shared_ptr<arrow::Table> table) {
    Parser::Expression childExpression = expression.expressions.at(0);
    std::string operatorName = childExpression.token.getLexeme();
    Parser::Expression leftChildExpression = childExpression.expressions.at(0);
    std::string columnName = leftChildExpression.token.getLexeme();
    Parser::Expression rightChildExpression = childExpression.expressions.at(1);
    std::string compareValue = rightChildExpression.token.getLexeme();

    return getWhere(operatorName, columnName, compareValue, table);
  }

  std::shared_ptr<arrow::Table> Interpreter::interpretColumns(Parser::Expression expression, std::shared_ptr<arrow::Table> table) {
    std::vector<std::string> columnNames = {};
    columnNames.reserve(expression.expressions.size());

    for (int index = 0; index < expression.expressions.size(); ++index) {
      columnNames.push_back(expression.expressions.at(index).token.getLexeme());
    }

    std::shared_ptr<arrow::Table> resultTable = this->selectColumns(columnNames, table);

    for (int index = 0; index < expression.expressions.size(); ++index) {
      std::optional<Parser::Expression> optionalAsExpression = getExpressionByTokenType(TokenType::AsOperator, expression.expressions.at(index).expressions);
      if (optionalAsExpression.has_value()) {
        resultTable = interpretAs(columnNames.at(index), optionalAsExpression.value(), resultTable);
      }
    }

    return resultTable;
  }

  std::shared_ptr<arrow::Table> Interpreter::interpretAtom(Parser::Expression expression, std::shared_ptr<arrow::Table> table) {
    return this->selectColumns({expression.token.getLexeme()}, table);
  }

  std::shared_ptr<arrow::Table> Interpreter::interpretDistinct(std::shared_ptr<arrow::Table> table) {
    return this->getDistinct(table);
  }

  std::shared_ptr<arrow::Table> Interpreter::interpretCount(Parser::Expression expression, std::shared_ptr<arrow::Table> table) {
    std::optional<Parser::Expression> optionalAllExpression = getExpressionByTokenType(TokenType::All, expression.expressions);
    if (!optionalAllExpression.has_value()) { // The all expression doesnt alter the table
      std::optional<Parser::Expression> optionalColumnsExpression = getExpressionByTokenType(TokenType::Columns, expression.expressions);
      std::optional<Parser::Expression> optionalAtomExpression = getExpressionByTokenType(TokenType::Atom, expression.expressions);
      
      if (optionalColumnsExpression.has_value()) {
        Parser::Expression columnsExpression = optionalColumnsExpression.value();
        table = interpretColumns(columnsExpression, table);
      } else if (optionalAtomExpression.has_value()) {
        Parser::Expression atomExpression = optionalAtomExpression.value();
        table = interpretAtom(atomExpression, table);
      } else {
        this->logger->log(tablog::CRITICAL, "Missing columns expression!");
        throw "Missing columns expression!";
      }
    }

    std::optional<Parser::Expression> optionalDistinctExpression = getExpressionByTokenType(TokenType::DistinctOperator, expression.expressions);
    if (optionalDistinctExpression.has_value()) {
      table = this->getDistinct(table);
    }
    
    std::shared_ptr<arrow::Table> resultTable = this->getCount(table);

    std::optional<Parser::Expression> optionalAsExpression = getExpressionByTokenType(TokenType::AsOperator, expression.expressions);
    if (optionalAsExpression.has_value()) {
      resultTable = interpretAs(resultTable->ColumnNames().at(0), optionalAsExpression.value(), resultTable);
    }
    
    return resultTable;
  }

  std::shared_ptr<arrow::Table> Interpreter::interpretMin(Parser::Expression expression, std::shared_ptr<arrow::Table> table) {
    std::optional<Parser::Expression> optionalDistinctExpression = getExpressionByTokenType(TokenType::DistinctOperator, expression.expressions);
    if (optionalDistinctExpression.has_value()) {
      table = this->getDistinct(table);
    }

    std::optional<Parser::Expression> optionalAtomExpression = getExpressionByTokenType(TokenType::Atom, expression.expressions);
     if (optionalAtomExpression.has_value()) {
      Parser::Expression atomExpression = optionalAtomExpression.value();
      table = interpretAtom(atomExpression, table);
    } else {
      this->logger->log(tablog::CRITICAL, "Missing columns expression!");
      throw "Missing atom expression!";
    }

    std::shared_ptr<arrow::Table> resultTable = this->getMin(table);

    std::optional<Parser::Expression> optionalAsExpression = getExpressionByTokenType(TokenType::AsOperator, expression.expressions);
    if (optionalAsExpression.has_value()) {
      resultTable = interpretAs(resultTable->ColumnNames().at(0), optionalAsExpression.value(), resultTable);
    }
    
    return resultTable;
  }
  
  std::shared_ptr<arrow::Table> Interpreter::interpretMax(Parser::Expression expression, std::shared_ptr<arrow::Table> table) {
    std::optional<Parser::Expression> optionalDistinctExpression = getExpressionByTokenType(TokenType::DistinctOperator, expression.expressions);
    if (optionalDistinctExpression.has_value()) {
      table = this->getDistinct(table);
    }

    std::optional<Parser::Expression> optionalAtomExpression = getExpressionByTokenType(TokenType::Atom, expression.expressions);
     if (optionalAtomExpression.has_value()) {
      Parser::Expression atomExpression = optionalAtomExpression.value();
      table = interpretAtom(atomExpression, table);
    } else {
      this->logger->log(tablog::CRITICAL, "Missing columns expression!");
      throw "Missing atom expression!";
    }

    std::shared_ptr<arrow::Table> resultTable = this->getMax(table);

    std::optional<Parser::Expression> optionalAsExpression = getExpressionByTokenType(TokenType::AsOperator, expression.expressions);
    if (optionalAsExpression.has_value()) {
      resultTable = interpretAs(resultTable->ColumnNames().at(0), optionalAsExpression.value(), resultTable);
    }
    
    return resultTable;
  }


  std::shared_ptr<arrow::Table> Interpreter::interpretAs(std::string originalColumnName, Parser::Expression expression, std::shared_ptr<arrow::Table> table) {
    return this->getRenamedTable(originalColumnName, expression.expressions.at(0).token.getLexeme(), table);
  }

  std::optional<Parser::Expression> Interpreter::getExpressionByTokenType(TokenType requestedTokenType, std::vector<Parser::Expression> expressions) {
    for (int index = 0; index < expressions.size(); ++index) {
      if (expressions.at(index).token.getType() == requestedTokenType)
        return expressions.at(index);
    }
    return {};
  }
}
