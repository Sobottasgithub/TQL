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

    int currentTokenType = tokens[cursor].getType();


    return {columnsExpression, cursor};    
  }

  std::tuple<Parser::Expression, int> Parser::parseDistinct(std::vector<Token> tokens, int cursor) {}
  std::tuple<Parser::Expression, int> Parser::parseCount(std::vector<Token> tokens, int cursor) {}
  std::tuple<Parser::Expression, int> Parser::parseFrom(std::vector<Token> tokens, int cursor) {}

}
