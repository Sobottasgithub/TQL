#include "../include/lexer.h"

#include "../include/token.h"
#include "../include/token_type.h"

#include <tablog.h>

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

namespace tql {
  Lexer::Lexer() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("Lexer", true);
    this->logger = logger;
  }

  std::vector<Token> Lexer::tokenize(const std::string query) {
    std::vector<Token> tokens = {};
    int startDelimiter = 0;
    int endDelimiter = 0;

    for (int index = 0; index < query.size(); index++) {
      std::string currentChar(1, query[index]);
      if (currentChar == " " || index == query.size()-1) {
        std::string previousChar(1, query[index - 1]);
        if (index == 0 || previousChar == " " && index != query.size()-1) {
          startDelimiter = index + 1;
          this->logger->log(tablog::DEBUG, "Space delimiter skipped at index " + std::to_string(index));
          continue;
        }
        
        endDelimiter = index;

        std::string tokenString = "";
        if (index == query.size()-1)
          tokenString = query.substr(startDelimiter, 1);
        else
          tokenString = query.substr(startDelimiter, endDelimiter-startDelimiter);

        if (std::find(Token::operators.begin(), Token::operators.end(), tokenString) != Token::operators.end()) {
          Token token(tokenString, TokenType::Operator);
          tokens.push_back(token);
        } else {
          Token token(tokenString, TokenType::Atom);
          tokens.push_back(token);
        }
        
        startDelimiter = index + 1;
      }
    }
    Token token("", TokenType::Eof);
    tokens.push_back(token);
    
    return tokens;
  }
}
