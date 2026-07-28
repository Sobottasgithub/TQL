#include "../include/parser.h"

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

  Parser::Expression Parser::parseTokens(std::vector<Token> tokens) {
    // Recursiv? 
  }

  std::tuple<float, float> Parser::getOperatorWeights(std::string operatorString) {
      if (operatorString == "SELECT" || operatorString == "UPDATE" ||
          operatorString == "DELETE" || operatorString == "INSERT")
        return {1.0, 2.0};
      if (operatorString == "WHERE" || operatorString == "FROM")
        return {1.0, 2.0};
      if (operatorString == "AND" || operatorString == "OR")
        return {1.0, 2.0};
      if (operatorString == "DISTINCT")
        return {1.0, 2.0};
      if (operatorString == "NOT")
        return {1.0, 2.0};
      if (operatorString == "=" || ">" || "<" || "IN")
        return {1.0, 2.0};
      if (operatorString == "BETWEEN")
        return {1.0, 2.0};
      if (operatorString == "ASC" || operatorString == "DESC")
        return {1.0, 2.0};
      if (operatorString == "ORDER")
        return {1.0, 2.0};
      if (operatorString == "BY")
        return {1.0, 2.0};
      else {
        this->logger->log(tablog::CRITICAL, "Unknown operation: " + operatorString);
        return {0, 0};
      }
  }
}
