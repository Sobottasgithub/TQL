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
          tokenString = query.substr(startDelimiter, (endDelimiter - startDelimiter) + 1);
        else
          tokenString = query.substr(startDelimiter, endDelimiter-startDelimiter);

        // check lastCharacter for delimiter
        Token delimiterToken("", TokenType::Delimiter);
        std::string lastCharacter(1, tokenString.at(tokenString.length() - 1));
        if (tokenIsInVector(Token::delimiters, lastCharacter)) {
          Token newDelimiterToken(lastCharacter, TokenType::Delimiter);
          delimiterToken = newDelimiterToken;
          tokenString = tokenString.substr(0, tokenString.length() - 1);
        }

        if (tokenIsInVector(Token::operators, tokenString)) {
          Token token(tokenString, TokenType::Operator);
          tokens.push_back(token);
        } else if (tokenString.compare("SELECT") == 0) {
          Token token(tokenString, TokenType::SelectOperator);
          tokens.push_back(token);
        } else if (tokenString.compare("DISTINCT") == 0) {
          Token token(tokenString, TokenType::DistinctOperator);
          tokens.push_back(token);
        } else if (tokenString.compare("COUNT") == 0) {
          Token token(tokenString, TokenType::CountOperator);
          tokens.push_back(token);
        } else if (tokenString.compare("AS") == 0) {
          Token token(tokenString, TokenType::AsOperator);
          tokens.push_back(token);
        } else if (tokenString.compare("FROM") == 0) {
          Token token(tokenString, TokenType::FromOperator);
          tokens.push_back(token);
        } else if (tokenString.compare("") != 0){
          Token token(tokenString, TokenType::Atom);
          tokens.push_back(token);
        } else {
          this->logger->log(tablog::CRITICAL, "Empty token!");
        }

        if (delimiterToken.getLexeme() != "") {
          tokens.push_back(delimiterToken);
        }
        
        startDelimiter = index + 1;
      }
    }
    Token token("", TokenType::Eof);
    tokens.push_back(token);
    
    return tokens;
  }

  bool Lexer::tokenIsInVector(std::vector<std::string> vector, std::string tokenString) {
    for (int index; index < vector.size(); index++) {
      if (vector[index].compare(tokenString) == 0)
        return true;
    }  
    return false;
  }    
}
