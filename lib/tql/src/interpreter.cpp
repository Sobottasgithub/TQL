#include "../include/interpreter.h"

#include "../include/token_type.h"

#include <arrow/table.h>
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

  void Interpreter::setSelectColumns(SelectColumns selectColumns) {
    this->selectColumns = selectColumns;
  }

  void Interpreter::setGetDistinct(GetDistinct getDistinct) {
    this->getDistinct = getDistinct;
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
      this->logger->log(tablog::CRITICAL, "Missing From expression!");
      throw "Missing From expression!";
    }

    //TODO: Interpret all the other functions
    

    // INFO: Interpret SELECT
    std::optional<Parser::Expression> optionalAllExpression = getExpressionByTokenType(TokenType::All, expression.expressions);
    if (!optionalAllExpression.has_value()) { // The all expression doesnt alter the table
      std::optional<Parser::Expression> optionalColumnsExpression = getExpressionByTokenType(TokenType::Columns, expression.expressions);
      std::optional<Parser::Expression> optionalCountExpression = getExpressionByTokenType(TokenType::CountOperator, expression.expressions);
    
      if (optionalColumnsExpression.has_value()) {
        Parser::Expression columnsExpression = optionalColumnsExpression.value();
        resultTable = interpretColumns(columnsExpression, resultTable);
      } else if (optionalColumnsExpression.has_value()) {
        Parser::Expression countExpression = optionalCountExpression.value();
      
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

  std::shared_ptr<arrow::Table> Interpreter::interpretColumns(Parser::Expression expression, std::shared_ptr<arrow::Table> table) {
    std::vector<std::string> columnNames = {};
    columnNames.reserve(expression.expressions.size());

    for (int index = 0; index < expression.expressions.size(); ++index) {
      columnNames.push_back(expression.expressions.at(index).token.getLexeme());
    }

    return this->selectColumns(columnNames, table);
  }

  std::shared_ptr<arrow::Table> Interpreter::interpretDistinct(std::shared_ptr<arrow::Table> table) {
    return this->getDistinct(table);
  }

  std::optional<Parser::Expression> Interpreter::getExpressionByTokenType(TokenType requestedTokenType, std::vector<Parser::Expression> expressions) {
    for (int index = 0; index < expressions.size(); ++index) {
      if (expressions.at(index).token.getType() == requestedTokenType)
        return expressions.at(index);
    }
    return {};
  }
}
