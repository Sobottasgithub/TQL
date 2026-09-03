#include "../include/parser.h"

#include "../include/token.h"
#include "../include/token_type.h"
#include "../include/lexer.h"

#include <tablog.h>

#include <vector>
#include <memory>
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
    Expression* currentOperatorExpression = nullptr;
    Expression* operandCache = nullptr;

    Parser::States currentState = States::AfterSelect;

    while(lexer->peek().getType() != TokenType::Eof) {
      switch (currentState) {
        case States::AfterSelect: {
          TokenType currentType = lexer->peek().getType();

          if (currentType == TokenType::All) {
            currentState = States::All;
          } else if (currentType == TokenType::Atom) {
            currentState = States::Column;
          } else if (currentType == TokenType::DistinctOperator) {
            currentState = States::Distinct;
          } else if (currentType == TokenType::CountOperator ||
                     currentType == TokenType::MinOperator ||
                     currentType == TokenType::MaxOperator ||
                     currentType == TokenType::SumOperator ||
                     currentType == TokenType::AvgOperator) {
            currentState = States::Aggregate;
          } else if (currentType == TokenType::FromOperator) {
            currentState = States::From;
          } else if (currentType == TokenType::WhereOperator) {
            currentState = States::Where;
          } else {
            // Return inner Select
            return expression;
          }
          continue;
        }

        case States::Column: {
          if (currentOperatorExpression == nullptr || currentOperatorExpression->token.getType() != TokenType::Columns) {
            Expression columnExpression;
            columnExpression.token = Token("", TokenType::Columns);
            expression.expressions.push_back(columnExpression);
            currentOperatorExpression = &expression.expressions.back();
          }
          if (lexer->peek().getType() != TokenType::Atom) {
            currentState = States::Invalid;
            continue;
          }
          
          Expression columnAtom;
          columnAtom.token = lexer->next();
          currentOperatorExpression->expressions.push_back(columnAtom);

          currentState = States::AfterColumn;
        }

        case States::AfterColumn: {
          if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme() == ",") {
            lexer->next();
            currentState = States::Column;
            continue;
          } else if (lexer->peek().getType() == TokenType::AsOperator) {
            currentState = States::As;
            continue;
          } else {
            currentState = States::AfterSelect;
            continue;
          }
        }

        case States::As: {
          Expression asExpression;
          asExpression.token = lexer->next();
          Expression* previousAtom = &currentOperatorExpression->expressions.back();
          previousAtom->expressions.push_back(asExpression);
          operandCache = &previousAtom->expressions.back();

          if (lexer->peek().getType() == TokenType::Atom) {
            currentState = States::AfterAs;
            continue;
          } else {
            currentState = States::Invalid;
            continue;
          }
        }

        case States::AggregateAs: {
          Expression asExpression;
          asExpression.token = lexer->next();
          currentOperatorExpression->expressions.push_back(asExpression);
          operandCache = &currentOperatorExpression->expressions.back();

          if (lexer->peek().getType() == TokenType::Atom) {
            currentState = States::AfterAs;
            continue;
          } else {
            currentState = States::Invalid;
            continue;
          }
        }

        case States::AfterAs: {
          Expression atomExpression;
          atomExpression.token = lexer->next();
          operandCache->expressions.push_back(atomExpression);
          operandCache = nullptr;

          if ((lexer->peek().getType() == TokenType::Delimiter
              && lexer->peek().getLexeme() == ",")
              || lexer->peek().getType() == TokenType::FromOperator) {
            currentState = States::AfterColumn;
            continue;
          } else {
            currentState = States::Invalid;
            continue;
          }
        }

        case States::All: {
          Expression allExpression;
          allExpression.token = lexer->next();
          expression.expressions.push_back(allExpression);
          currentState = States::AfterSelect;
          continue;
        }

        case States::Distinct: {
          Expression distinctExpression;
          distinctExpression.token = lexer->next();
          expression.expressions.push_back(distinctExpression);
          currentState = States::AfterSelect;
          continue;
        }

        case States::Aggregate: {
          Expression aggregateExpression;
          aggregateExpression.token = lexer->next();
          expression.expressions.push_back(aggregateExpression);
          currentOperatorExpression = &expression.expressions.back();

          if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme() == "(") {
            lexer->next();

            if (lexer->peek().getType() == TokenType::DistinctOperator) {
              currentState = States::AggregateDistinct;
              continue;
            } else if (lexer->peek().getType() == TokenType::Atom) {
              currentState = States::AfterAggregate;
              continue;
            } else if (lexer->peek().getType() == TokenType::All && aggregateExpression.token.getType() == TokenType::CountOperator) {
              // Only the count operator can be matched with the * operator, though the afterAggregate state isnt type dependent
              currentState = States::AfterAggregate;
              continue;
            }
          }
          currentState = States::Invalid;
          continue;
        }

        case States::AggregateDistinct: {
          Expression distinctExpression;
          distinctExpression.token = lexer->next();
          currentOperatorExpression->expressions.push_back(distinctExpression);

          if (lexer->peek().getType() == TokenType::Atom) {
            currentState = States::AfterAggregate;
            continue;
          }
          currentState = States::Invalid;
          continue;
        }

        case States::AfterAggregate: {
          Expression atomExpression;
          atomExpression.token = lexer->next();
          currentOperatorExpression->expressions.push_back(atomExpression);

          if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme() == ")") {
            lexer->next();

            if (lexer->peek().getType() == TokenType::AsOperator) {
              currentState = States::AggregateAs;
              continue;  
            }
            
            currentState = States::AfterSelect;
            continue;
          }
          currentState = States::Invalid;
          continue;
        }

        case States::From: {
          Expression fromExpression;
          fromExpression.token = lexer->next();

          // Push first, then capture pointer to the vector's element
          expression.expressions.push_back(fromExpression);
          currentOperatorExpression = &expression.expressions.back();

          currentState = States::AfterFrom;
          continue;
        }

        case States::AfterFrom: {
          if (lexer->peek().getType() == TokenType::Atom) {
            Expression atomExpression;
            atomExpression.token = lexer->next();

            if (currentOperatorExpression) {
              currentOperatorExpression->expressions.push_back(atomExpression);
            }

            currentState = States::AfterSelect;
          } else if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme() == "(") {
            lexer->next();

            Expression selectExpression;
            if (lexer->peek().getType() == TokenType::SelectOperator) {
              selectExpression.token = lexer->next();
              Expression parsedSelectExpression = parseSelect(lexer);
              selectExpression.expressions = parsedSelectExpression.expressions;
              currentOperatorExpression->expressions.push_back(selectExpression);
            } else {
              throw std::invalid_argument("Expected SELECT");
              continue;
            }

            if (lexer->peek().getType() != TokenType::Delimiter && lexer->peek().getLexeme() != ")") {
              currentState = States::Invalid;
              continue;
            }
            lexer->next();
          } else {
            currentState = States::Invalid;
          }
          continue;
        }

        case States::Where: {
          Expression whereExpression;
          whereExpression.token = lexer->next();
          expression.expressions.push_back(whereExpression);
          currentOperatorExpression = &expression.expressions.back();

          if (lexer->peek().getType() == TokenType::Atom) {
            currentState = States::AfterWhereInfixAtom;
          } else {
            currentState = States::AfterSelect;
          }
        }

        case States::AfterWhereInfixAtom: {
          if (operandCache == nullptr) {
            // Static here because we only want to store the pointer
            // without creating a dangling pointer
            static Expression atomExpression;
            atomExpression.token = lexer->next();
            operandCache = &atomExpression;
            TokenType peekType = lexer->peek().getType();
            if (peekType == TokenType::EqualOperator ||
                peekType == TokenType::GreaterOperator ||
                peekType == TokenType::SmallerOperator ||
                peekType == TokenType::GreaterEqualOperator ||
                peekType == TokenType::SmallerEqualOperator ||
                peekType == TokenType::UnequalOperator) {
              currentState = States::AfterWhereInfixOperator;
            } else {
              currentState = States::Invalid;
            }
            continue;
          } else if (operandCache->token.getType() != TokenType::Atom) {
            Expression atomExpression;
            atomExpression.token = lexer->next();
            operandCache->expressions.push_back(atomExpression);
            operandCache = nullptr;
            currentState = States::Where;
            continue;
          } else {
            currentState = States::Invalid;
            continue;
          }
        }

        case States::AfterWhereInfixOperator: {
          Expression infixOperator;
          infixOperator.token = lexer->next();
          infixOperator.expressions.push_back(*operandCache);
          currentOperatorExpression->expressions.push_back(infixOperator);
          operandCache = &currentOperatorExpression->expressions.back();

          if (lexer->peek().getType() == TokenType::Atom) {
            currentState = States::AfterWhereInfixAtom;
            continue;
          } else {
            currentState = States::Invalid;
            continue;
          }
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
