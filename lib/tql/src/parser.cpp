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
    Expression* currentExpression = nullptr;
    Expression* currentOperatorExpression = nullptr;

    Parser::States currentState = States::Start;
    const int collectionSize = lexer.getTokenCollectionSize();

    for (int tokenIndex = 0; tokenIndex < collectionSize;) {
        switch (currentState) {
            case States::Start: {
                if (lexer.peek().getType() == TokenType::SelectOperator) {
                    expression.token = lexer.next();
                    currentExpression = &expression;
                    currentState = States::AfterSelect;
                    tokenIndex++;
                } else {
                    throw std::invalid_argument("Expected SELECT at index " + std::to_string(tokenIndex));
                }
                break;
            }

            case States::AfterSelect: {
                TokenType currentType = lexer.peek().getType();
                logger->log(tablog::DEBUG, "AfterSelect: " + lexer.peek().getTypeAsString());

                if (currentType == TokenType::All) {
                    currentState = States::All;
                } else if (currentType == TokenType::Atom) {
                    currentState = States::Column;
                } else if (currentType == TokenType::FromOperator) {
                    currentState = States::From;
                } else if (currentType == TokenType::Eof) {
                    logger->log(tablog::DEBUG, "EOF");
                    tokenIndex++;
                } else {
                    throw std::invalid_argument("Unexpected token in AfterSelect: " + lexer.peek().getTypeAsString());
                }
                break;
            }

            case States::All: {
                Expression allExpression;
                allExpression.token = lexer.next();
                currentExpression->expressions.push_back(allExpression);
                currentState = States::AfterSelect;
                tokenIndex++;
                break;
            }

            case States::From: {
                logger->log(tablog::DEBUG, "FROM");
                Expression fromExpression;
                fromExpression.token = lexer.next();
            
                // Push first, then capture pointer to the vector's element
                currentExpression->expressions.push_back(fromExpression);
                currentOperatorExpression = &currentExpression->expressions.back();

                currentState = States::AfterFrom;
                tokenIndex++;
                break;
            }

            case States::AfterFrom: {
                logger->log(tablog::DEBUG, "AfterFrom");
                Expression atomExpression;
                atomExpression.token = lexer.next();

                if (currentOperatorExpression) {
                  currentOperatorExpression->expressions.push_back(atomExpression);
                }

                currentState = States::AfterSelect;
                tokenIndex++;
                break;
            }

            default: {
              throw std::invalid_argument("Invalid State or Token! " + std::to_string(tokenIndex) + " " + lexer.peek().getTypeAsString());
            }
        }
    }
    return expression;
  }
}
