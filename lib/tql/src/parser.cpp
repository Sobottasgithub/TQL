#include "../include/parser.h"

#include "../include/token.h"
#include "../include/token_type.h"
#include "../include/lexer.h"

#include <tablog.h>

#include <tuple>
#include <vector>
#include <memory>

namespace tql {
  Parser::Parser() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("Parser", true);
    this->logger = logger;
  }

  Parser::Expression Parser::parse(Lexer lexer) {
    if (lexer.peek().getType() == TokenType::SelectOperator) {
        return parseSelect(&lexer);
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected token of type Dml!");
    }
  }

  Parser::Expression Parser::parseSelect(Lexer* lexer) {
    Parser::Expression selectExpression;
    if (lexer->peek().getType() == TokenType::SelectOperator) {
      selectExpression.token = lexer->next();
      
      if (lexer->peek().getType() == TokenType::DistinctOperator) {
        Expression distinctExpression;
        distinctExpression.token = lexer->next();
        selectExpression.expressions.push_back(distinctExpression);
        
        if (lexer->peek().getType() == TokenType::Atom) {
          Parser::Expression result = parseColumns(lexer);
          selectExpression.expressions.push_back(result);
        } else if (lexer->peek().getType() == TokenType::All) {
          Expression allExpression;
          allExpression.token = lexer->next();
          selectExpression.expressions.push_back(allExpression);
        } else {
          this->logger->log(tablog::ERROR, "Bad Token! Expected atom or *!");
        }
      } else {
        int tokenType = lexer->peek().getType();
        if (tokenType == TokenType::Atom) {
          Parser::Expression result = parseColumns(lexer);
          selectExpression.expressions.push_back(result);
        } else if (tokenType == TokenType::All) {
          Expression allExpression;
          allExpression.token = lexer->next();
          selectExpression.expressions.push_back(allExpression);
        } else if (tokenType == TokenType::CountOperator) {
          Parser::Expression result = parseCount(lexer);
          selectExpression.expressions.push_back(result);
        } else {
          this->logger->log(tablog::ERROR, "Bad Token! Expected atom or count operator!");
        }
      }
      
      Parser::Expression fromResult = parseFrom(lexer);
      selectExpression.expressions.push_back(fromResult);

      return selectExpression;
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected token of type SELECT!");
    }
    return selectExpression;
  }

  Parser::Expression Parser::parseColumns(Lexer* lexer) {
    Expression columnsExpression;
    columnsExpression.token = Token("", TokenType::Columns);

    do {
      if (lexer->peek().getLexeme().compare(",") == 0)
        lexer->next();
      
      if (lexer->peek().getType() == TokenType::Atom) {
        Parser::Expression parsedAtom = parseAtom(lexer);
        columnsExpression.expressions.push_back(parsedAtom);
      }
    } while (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme().compare(",") == 0);
    
    return columnsExpression;    
  }

  Parser::Expression Parser::parseAtom(Lexer* lexer) {
    Expression atomExpression;

    if (lexer->peek().getType() == TokenType::Atom) {
        atomExpression.token = lexer->next();
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected Atom!");
    }

    if (lexer->peek().getType() == TokenType::AsOperator) {
      Parser::Expression parsedAs = parseAs(lexer);
      atomExpression.expressions.push_back(parsedAs);
    }

    return atomExpression;
  }

  Parser::Expression Parser::parseAs(Lexer* lexer) {
    Expression asExpression;

    if (lexer->peek().getType() == TokenType::AsOperator) {
      asExpression.token = lexer->next();
      
      Parser::Expression parsedAtom = parseAtom(lexer);
      asExpression.expressions.push_back(parsedAtom);
    }

    return asExpression;
  }
  
  Parser::Expression Parser::parseCount(Lexer* lexer) {
    Expression countExpression;

    if (lexer->peek().getType() == TokenType::CountOperator) {
      countExpression.token = lexer->next();

      // INFO: Delimiters are not added to parser tree
      if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme().compare("(") == 0) {
        lexer->next();
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected delimiter: '(' !");
      }

      if (lexer->peek().getType() == TokenType::DistinctOperator) {
        Expression distinctExpression;
        distinctExpression.token = lexer->next();
      }

      if (lexer->peek().getType() == TokenType::Atom) {
        Parser::Expression result = parseColumns(lexer);
        countExpression.expressions.push_back(result);
      } else if (lexer->peek().getType() == TokenType::All) {
        Expression allExpression;
        allExpression.token = lexer->next();
        countExpression.expressions.push_back(allExpression);
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Count expected atom or *!");
      }

      if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme().compare(")") == 0) {
        lexer->next();
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected delimiter: ')' !");
      }
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected Count!");
    }

    return countExpression;
  }
  
  Parser::Expression Parser::parseFrom(Lexer* lexer) {
    Expression fromExpression;

    if (lexer->peek().getType() == TokenType::FromOperator) {
      fromExpression.token = lexer->next();
      if (lexer->peek().getType() == TokenType::Atom) {
        Parser::Expression parsedAtom = parseAtom(lexer);
        fromExpression.expressions.push_back(parsedAtom);
      } else if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme().compare("(") == 0) {
          lexer->next();
          if (lexer->peek().getType() == TokenType::SelectOperator) {
            Parser::Expression selectResult = parseSelect(lexer);
            fromExpression.expressions.push_back(selectResult);
            lexer->next();
            
            if (lexer->peek().getLexeme().compare(")") != 0) {
              this->logger->log(tablog::ERROR, "Bad Token! Expected closing Delimiter after Select operator in From operation!");
            }
          } else {
            this->logger->log(tablog::ERROR, "Bad Token! Expected Select operator after Delimiter!");
          }
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected Atom or Delimiter after From operator!");
      }
    }
    return fromExpression;
  }
}
