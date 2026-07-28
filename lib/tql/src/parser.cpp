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
    return parseTokens(tokens, 0, 0.0);
  }

  Parser::Expression Parser::parseTokens(std::vector<Token> tokens, int cursor, float minWeight) {
    if (cursor >= tokens.size()) return {};
    Token token = tokens[cursor++];

    Expression leftExpression;
    if (token.getType() == TokenType::Operator) {
      // Unary operator
      float prefixWeight = std::get<0>(getOperatorWeight(token.getLexeme()));
      Expression operand = parseTokens(tokens, cursor, prefixWeight);
      leftExpression = Expression{token, {operand}};
    } else {
      leftExpression = Expression{token};
    }

    while (cursor < tokens.size()) {
      Token nextToken = tokens[cursor];
      if (nextToken.getType() == TokenType::Eof)
        break;
      if (nextToken.getType() == TokenType::Operator) {
        auto [leftWeight, rightWeight] = getOperatorWeight(nextToken.getLexeme());

        if (leftWeight < minWeight)
          break;

        cursor++;
        Expression rightExpression = parseTokens(tokens, cursor, rightWeight);
        leftExpression = Expression{nextToken, {leftExpression, rightExpression}};
      } else {
        break;
      }
    }

    return leftExpression;
  }

  std::tuple<float, float> Parser::getOperatorWeight(std::string operatorString) {
    if (operatorString == "SELECT" || operatorString == "UPDATE" ||
        operatorString == "DELETE" || operatorString == "INSERT")
      return {1.0, 1.1};
    if (operatorString == "WHERE" || operatorString == "FROM")
      return {1.0, 1.1};
    if (operatorString == "AND" || operatorString == "OR")
      return {2.1, 2.1};
    if (operatorString == "DISTINCT")
      return {1.0, 2.0};
    if (operatorString == "NOT")
      return {1.0, 2.0};
    if (operatorString == "=" || ">" || "<" || "IN")
      return {2.1, 2.1};
    if (operatorString == "BETWEEN")
      return {1.0, 1.1};
    if (operatorString == "ASC" || operatorString == "DESC")
      return {1.1, 1.0};
    if (operatorString == "ORDER")
      return {1.0, 1.1};
    if (operatorString == "BY")
      return {1.1, 1.1};
    else {
      this->logger->log(tablog::CRITICAL, "Unknown operation: " + operatorString);
      return {0, 0};
    }
  }
}
