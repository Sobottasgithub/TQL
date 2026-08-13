#include "../include/lexer.h"

#include "../include/token.h"
#include "../include/token_type.h"

#include <tablog.h>

#include <memory>
#include <vector>
#include <string>

namespace tql {
  Lexer::Lexer() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("Lexer", true);
    this->logger = logger;
  }

  void Lexer::tokenize(const std::string query) {
    std::vector<Token> tokens = {};

    


    
    Token token("", TokenType::Eof);
    tokens.push_back(token);
    
    this->tokens = tokens;
  }

  Token Lexer::peek() {
    if (this->tokens.size() > 0) {
      return this->tokens.at(0);
    }
    return Token();
  }

  Token Lexer::next() {
    if (this->tokens.size() > 0) {
      Token firstToken = this->tokens.at(0);
      this->tokens.erase(this->tokens.begin());
      return firstToken;
    }
    return Token();

  }

  TokenType Lexer::getTokenStringAsType(std::string tokenString) {
    if (tokenString == "SELECT")
      return TokenType::SelectOperator;
    else if (tokenString == "DISTINCT")
      return TokenType::DistinctOperator;
    else if (tokenString == "AS")
      return TokenType::AsOperator;
    else if (tokenString == "COUNT")
      return TokenType::CountOperator;
    else if (tokenString == "MIN")
      return TokenType::MinOperator;
    else if (tokenString == "MAX")
      return TokenType::MaxOperator;
    else if (tokenString == "SUM")
      return TokenType::SumOperator;
    else if (tokenString == "AVG")
      return TokenType::AvgOperator;
    else if (tokenString == "FROM")
      return TokenType::FromOperator;
    else if (tokenString == "*")
      return TokenType::All;
    else
      return TokenType::Invalid;
  }
}
