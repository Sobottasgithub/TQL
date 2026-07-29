#include "../include/parser.h"

#include "../include/token.h"
#include "../include/token_type.h"

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

  Parser::Expression Parser::parse(std::vector<Token> tokens) {
    return parseTokens(tokens, 0);
  }

  Parser::Expression Parser::parseTokens(std::vector<Token> tokens, int cursor) {
    if (tokens[cursor].getType() == TokenType::SelectOperator) {
        return std::get<0>(parseSelect(tokens, cursor));
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected token of type Dml!");
    }
  }

  std::tuple<Parser::Expression, int> Parser::parseSelect(std::vector<Token> tokens, int cursor) {
    Parser::Expression selectExpression;
    if (tokens[cursor].getType() == TokenType::SelectOperator) {
      selectExpression.token = tokens[cursor];
      cursor++;
      int tokenType = tokens[cursor].getType();
      if (tokenType == TokenType::Atom || tokenType == TokenType::DistinctOperator) {
        std::tuple<Parser::Expression, int> result = parseColumns(tokens, cursor);
        selectExpression.expressions.push_back(std::get<0>(result));
        cursor = std::get<1>(result);
      } else if (tokenType == TokenType::CountOperator) {
        std::tuple<Parser::Expression, int> result = parseCount(tokens, cursor);
        selectExpression.expressions.push_back(std::get<0>(result));
        cursor = std::get<1>(result);
      } else {
            this->logger->log(tablog::ERROR, "Bad Token! Expected atom or distinct operator!");
      }

      cursor++;
      parseFrom(tokens, cursor);

      return {selectExpression, cursor};
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected token of type SELECT!");
    }
    return {selectExpression, cursor};
  }

  std::tuple<Parser::Expression, int>  Parser::parseColumns(std::vector<Token> tokens, int cursor) {
    Expression columnsExpression;
    columnsExpression.token = Token("", TokenType::Columns);

    do {
      if (tokens[cursor].getLexeme().compare(",") == 0)
        cursor++;
      
      if (tokens[cursor].getType() == TokenType::Atom) {
        std::tuple<Parser::Expression, int> parsedAtom = parseAtom(tokens, cursor);
        columnsExpression.expressions.push_back(std::get<0>(parsedAtom));
        cursor = std::get<1>(parsedAtom);
      } else if (tokens[cursor].getType() == TokenType::DistinctOperator) {
        std::tuple<Parser::Expression, int> parsedDistinct = parseDistinct(tokens, cursor);
        columnsExpression.expressions.push_back(std::get<0>(parsedDistinct));
        cursor = std::get<1>(parsedDistinct);
      }
      cursor++;
    } while (tokens[cursor].getType() == TokenType::Delimiter && tokens[cursor].getLexeme().compare(",") == 0);
    
    return {columnsExpression, cursor};    
  }

  std::tuple<Parser::Expression, int> Parser::parseAtom(std::vector<Token> tokens, int cursor) {
    Expression atomExpression;

    if (tokens[cursor].getType() == TokenType::Atom) {
        atomExpression.token = tokens[cursor];
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected Atom!");
    }

    if (tokens[cursor + 1].getType() == TokenType::AsOperator) {
      cursor++;
      std::tuple<Parser::Expression, int> parsedAs = parseAs(tokens, cursor);
      atomExpression.expressions.push_back(std::get<0>(parsedAs));
      cursor = std::get<1>(parsedAs);
    }

    return {atomExpression, cursor};
  }

  std::tuple<Parser::Expression, int> Parser::parseAs(std::vector<Token> tokens, int cursor) {
    Expression asExpression;

    if (tokens[cursor].getType() == TokenType::AsOperator) {
      asExpression.token = tokens[cursor];
      cursor++;
      
      std::tuple<Parser::Expression, int> parsedAs = parseAtom(tokens, cursor);
      asExpression.expressions.push_back(std::get<0>(parsedAs));
      cursor = std::get<1>(parsedAs);
    }

    return {asExpression, cursor};
  }
  
  std::tuple<Parser::Expression, int> Parser::parseDistinct(std::vector<Token> tokens, int cursor) {
    Expression distinctExpression;

    if (tokens[cursor].getType() == TokenType::DistinctOperator) {
      distinctExpression.token = tokens[cursor];

      cursor++;
      if (tokens[cursor].getType() == TokenType::Atom) {
        std::tuple<Parser::Expression, int> parsedAtom = parseAtom(tokens, cursor);
        distinctExpression.expressions.push_back(std::get<0>(parsedAtom));
        cursor = std::get<1>(parsedAtom);
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected Atom after Distinct operator!");
      }
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected Distinct!");
    }

    return {distinctExpression, cursor};
  }
  
  std::tuple<Parser::Expression, int> Parser::parseCount(std::vector<Token> tokens, int cursor) {
    Expression countExpression;

    if (tokens[cursor].getType() == TokenType::CountOperator) {
      countExpression.token = tokens[cursor];

      // INFO: Delimiters are not added to parser tree
      cursor++;
      if (tokens[cursor].getType() != TokenType::Delimiter && tokens[cursor].getLexeme().compare("(") != 0) {
        this->logger->log(tablog::ERROR, "Bad Token! Expected delimiter: '(' !");
      }

      cursor++;
      if (tokens[cursor].getType() == TokenType::Atom || tokens[cursor].getType() == TokenType::DistinctOperator) {
        std::tuple<Parser::Expression, int> result = parseColumns(tokens, cursor);
        countExpression.expressions.push_back(std::get<0>(result));
        cursor = std::get<1>(result);
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Count expected atom or distinct!");
      }

      cursor++;
      if (tokens[cursor].getType() != TokenType::Delimiter && tokens[cursor].getLexeme().compare(")") != 0) {
        this->logger->log(tablog::ERROR, "Bad Token! Expected delimiter: ')' !");
      }
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected Count!");
    }



    return {countExpression, cursor};
  }
  
  std::tuple<Parser::Expression, int> Parser::parseFrom(std::vector<Token> tokens, int cursor) {
    Expression fromExpression;

    if (tokens[cursor].getType() == TokenType::FromOperator) {
      fromExpression.token = tokens[cursor];
      cursor++;
      if (tokens[cursor].getType() == TokenType::Atom) {
        std::tuple<Parser::Expression, int> parsedAtom = parseAtom(tokens, cursor);
        fromExpression.expressions.push_back(std::get<0>(parsedAtom));
        cursor = std::get<1>(parsedAtom);
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected Atom after From operator!");
      }
    }
    return {fromExpression, cursor};
  }
}
