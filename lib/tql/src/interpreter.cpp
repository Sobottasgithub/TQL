#include "../include/interpreter.h"

#include "../include/token_type.h"

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

  std::shared_ptr<arrow::Table> Interpreter::interpret(Parser::Expression expressionTree) {
    if (expressionTree.token.getType() == TokenType::SelectOperator) {
      return interpretSelect(expressionTree);
    } else {
      this->logger->log(tablog::CRITICAL, "Unknown DML operation!");
    }
    return nullptr;
  }

  std::shared_ptr<arrow::Table> Interpreter::interpretSelect(Parser::Expression expression) {
    std::optional<Parser::Expression> optionalFromExpression = getExpressionByTokenType(TokenType::FromOperator, expression.expressions);
    if (optionalFromExpression.has_value()) {
      Parser::Expression fromExpression = optionalFromExpression.value();

      // TODO: CHANGE THIS LATER AS MORE FUNCTIONS GET IMPLEMENTED!!!!
      return interpretFrom(fromExpression);
    } else {
      this->logger->log(tablog::CRITICAL, "Missing From expression!");
      throw "Missing From expression!";
    }
  }

  std::shared_ptr<arrow::Table> Interpreter::interpretFrom(Parser::Expression expression) {
    Parser::Expression childExpression = expression.expressions.at(0);
    TokenType childTokenType = childExpression.token.getType();
    if (childTokenType == TokenType::Atom) {
      return this->openFile(childExpression.token.getLexeme());
    } else if (childTokenType == TokenType::SelectOperator) {
      interpretSelect(childExpression);
    } else {
      this->logger->log(tablog::CRITICAL, "Undefined behaviour in from for token type: " + childExpression.token.getTypeAsString());
      throw "Undefined behaviour in from for token type";
    }
    return nullptr;
  }

  std::optional<Parser::Expression> Interpreter::getExpressionByTokenType(TokenType requestedTokenType, std::vector<Parser::Expression> expressions) {
    for (int index = 0; index < expressions.size(); ++index) {
      if (expressions.at(index).token.getType() == requestedTokenType)
        return expressions.at(index);
    }
    return {};
  }
}
