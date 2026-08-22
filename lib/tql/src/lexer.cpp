#include "../include/lexer.h"

#include "../include/token.h"
#include "../include/token_type.h"

#include <tablog.h>

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

  void Lexer::tokenize(const std::string query) {
    std::vector<Token> tokens = {};

    std::string currentToken = {""};
    for (int index = 0; index < query.size(); ++index) {
      std::string currentChar(1, query[index]);
      if (currentChar.compare(" ") == 0) {
        if (currentToken.size() == 0) {
          currentToken = "";
          continue;
        }
        Token token(currentToken, TokenType::Atom);
        tokens.push_back(token);
        currentToken = "";
        continue;
      }

      TokenType currentCharTokenType = getTokenStringAsType(currentChar);
      if (currentCharTokenType != TokenType::Invalid && getTokenStringAsType(currentToken) == TokenType::Invalid) {
        if(currentToken.size() > 0) {
          Token tokenAtom(currentToken, TokenType::Atom);
          tokens.push_back(tokenAtom);
          currentToken = "";
        }

        Token token(currentChar, currentCharTokenType);
        tokens.push_back(token);
        continue;
      }
      
      currentToken += currentChar;
      TokenType currentTokenType = getTokenStringAsType(currentToken);
      if (currentTokenType != TokenType::Invalid) {
        Token token(currentToken, currentTokenType);
        tokens.push_back(token);
        currentToken = "";
      } else {
        if (tokens.size() <= 0)
          continue;

        Token lastToken = tokens.back();
        std::string concatedTokenString = lastToken.getLexeme() + currentToken;
        TokenType concatedTokenType = getTokenStringAsType(concatedTokenString);
        if (concatedTokenType != TokenType::Invalid) {
          tokens.pop_back();
          Token token(currentToken, concatedTokenType);
          tokens.push_back(token);
          currentToken = "";
        }
      }
    }    

    if (currentToken.size() > 0) {
      Token token(currentToken, TokenType::Atom);
      tokens.push_back(token);
    }
    
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

  int Lexer::getTokenCollectionSize() {
    return this->tokens.size();
  }

  TokenType Lexer::getTokenStringAsType(std::string tokenString) {
    std::transform(tokenString.begin(), tokenString.end(), tokenString.begin(), ::toupper);
    if (tokenString.compare("SELECT") == 0)
      return TokenType::SelectOperator;
    else if (tokenString.compare("DISTINCT") == 0)
      return TokenType::DistinctOperator;
    else if (tokenString.compare("AS") == 0)
      return TokenType::AsOperator;
    else if (tokenString.compare("COUNT") == 0)
      return TokenType::CountOperator;
    else if (tokenString.compare("MIN") == 0)
      return TokenType::MinOperator;
    else if (tokenString.compare("MAX") == 0)
      return TokenType::MaxOperator;
    else if (tokenString.compare("SUM") == 0)
      return TokenType::SumOperator;
    else if (tokenString.compare("AVG") == 0)
      return TokenType::AvgOperator;
    else if (tokenString.compare("FROM") == 0)
      return TokenType::FromOperator;
    else if (tokenString.compare("*") == 0)
      return TokenType::All;
    else if (tokenString.compare("WHERE") == 0)
      return TokenType::WhereOperator;
    else if (tokenString.compare("=") == 0)
      return TokenType::EqualOperator;
    else if (tokenString.compare("(") == 0
             || tokenString.compare(")") == 0
             || tokenString.compare(",") == 0)
      return TokenType::Delimiter;
    else
      return TokenType::Invalid;
  }
}
