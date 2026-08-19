#include "../include/parser.h"

#include "../include/token.h"
#include "../include/token_type.h"
#include "../include/lexer.h"

#include <tablog.h>

#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace tql {
  Parser::Parser() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("Parser", true);
    this->logger = logger;
  }


  Parser::Expression Parser::parse(Lexer lexer) {
    Expression expression;
    if (lexer.peek().getType() == TokenType::SelectOperator) {
      expression.token = lexer.next();
      Expression parsedSelectExpression = parseSelect(&lexer);
      expression.expressions = parsedSelectExpression.expressions;
    } else {
      throw std::invalid_argument("Expected SELECT");
    }
    return expression;
  }

  Parser::Expression Parser::parseSelect(Lexer* lexer) {
    Expression expression;
    Expression* currentExpression = &expression;
    Expression* currentOperatorExpression = nullptr;

    Parser::States currentState = States::AfterSelect;

    while(lexer->peek().getType() != TokenType::Eof) {
      switch (currentState) {
        case States::AfterSelect: {
          TokenType currentType = lexer->peek().getType();
          logger->log(tablog::DEBUG, "AfterSelect: " + lexer->peek().getTypeAsString());

          if (currentType == TokenType::All) {
            currentState = States::All;
          } else if (currentType == TokenType::Atom) {
            currentState = States::Column;
          } else if (currentType == TokenType::FromOperator) {
            currentState = States::From;
          } else if (currentType == TokenType::Eof) {
            logger->log(tablog::DEBUG, "EOF");
            // Return when end of File is reached
            return expression;
          } else {
            // Return inner Select
            return expression;
          }
          continue;
        }

        case States::All: {
          Expression allExpression;
          allExpression.token = lexer->next();
          currentExpression->expressions.push_back(allExpression);
          currentState = States::AfterSelect;
          continue;
        }

        case States::From: {
          logger->log(tablog::DEBUG, "FROM");
          Expression fromExpression;
          fromExpression.token = lexer->next();

          // Push first, then capture pointer to the vector's element
          currentExpression->expressions.push_back(fromExpression);
          currentOperatorExpression = &currentExpression->expressions.back();

          currentState = States::AfterFrom;
          continue;
        }

        case States::AfterFrom: {
          logger->log(tablog::DEBUG, "AfterFrom");

          if (lexer->peek().getType() == TokenType::Atom) {
            Expression atomExpression;
            atomExpression.token = lexer->next();

            if (currentOperatorExpression) {
              currentOperatorExpression->expressions.push_back(atomExpression);
            }

            currentState = States::AfterSelect;
          } else if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme() == "(") {
            lexer->next();

            
          } else {
            currentState = States::Invalid;
          }
          continue;
        }
        
        case States::Invalid: {
          throw std::invalid_argument("Invalid State or Token! " + lexer->peek().getTypeAsString());
        }
        default: {
          throw std::invalid_argument("Invalid State or Token! " + lexer->peek().getTypeAsString());
        }
      }
    }
    return expression;
  }
}
