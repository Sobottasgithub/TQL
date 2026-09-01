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
      // Create atom
      if (!currentChar.compare(" ")) {
        if (currentToken.size() == 0) {
          currentToken = "";
          continue;
        }
        Token token(currentToken, TokenType::Atom);
        tokens.push_back(token);
        currentToken = "";
        continue;
      }

      // Current char is an operator, current token is an atom
      TokenType currentCharTokenType = getTokenStringAsType(currentChar);
      if (currentCharTokenType != TokenType::Invalid && getTokenStringAsType(currentToken) == TokenType::Invalid) {
        if(currentToken.size() > 0) {
          Token tokenAtom(currentToken, TokenType::Atom);
          tokens.push_back(tokenAtom);
          currentToken = "";
        }

        // Check if current char + previous Token = valid Operator
        if (tokens.size() > 0) {
          Token lastToken = tokens.back();
          std::string concatedTokenString = lastToken.getLexeme() + currentChar;
          TokenType concatedTokenType = getTokenStringAsType(concatedTokenString);
          if (concatedTokenType != TokenType::Invalid) {
            tokens.pop_back();
            Token token(concatedTokenString, concatedTokenType);
            tokens.push_back(token);
            continue;
          }
        }

        Token token(currentChar, currentCharTokenType);
        tokens.push_back(token);
        continue;
      }

      // Current Token is an operator
      currentToken += currentChar;
      TokenType currentTokenType = getTokenStringAsType(currentToken);
      if (currentTokenType != TokenType::Invalid) {
        if (tokens.size() > 0) {
          // Test if previous token + current token = valid Operator
          Token lastToken = tokens.back();
          std::string concatedTokenString = lastToken.getLexeme() + currentToken;
          TokenType concatedTokenType = getTokenStringAsType(concatedTokenString);
          if (concatedTokenType != TokenType::Invalid) {
            tokens.pop_back();
            Token token(concatedTokenString, concatedTokenType);
            tokens.push_back(token); 
          } else {
            Token token(currentToken, currentTokenType);
            tokens.push_back(token); 
          }
        } else {
          Token token(currentToken, currentTokenType);
          tokens.push_back(token);
        }
        currentToken = "";
      } else {
        // Current Token is not an operator
        if (tokens.size() <= 0)
          continue;

        Token lastToken = tokens.back();
        std::string concatedTokenString = lastToken.getLexeme() + currentToken; // Combine with previous Token
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
    // std::transform(tokenString.begin(), tokenString.end(), tokenString.begin(), ::toupper);
    if (!tokenString.compare("SELECT"))
      return TokenType::SelectOperator;
    else if (!tokenString.compare("DISTINCT"))
      return TokenType::DistinctOperator;
    else if (!tokenString.compare("AS"))
      return TokenType::AsOperator;
    else if (!tokenString.compare("COUNT"))
      return TokenType::CountOperator;
    else if (!tokenString.compare("MIN"))
      return TokenType::MinOperator;
    else if (!tokenString.compare("MAX"))
      return TokenType::MaxOperator;
    else if (!tokenString.compare("SUM"))
      return TokenType::SumOperator;
    else if (!tokenString.compare("AVG"))
      return TokenType::AvgOperator;
    else if (!tokenString.compare("FROM"))
      return TokenType::FromOperator;
    else if (!tokenString.compare("*"))
      return TokenType::All;
    else if (!tokenString.compare("WHERE"))
      return TokenType::WhereOperator;
    else if (!tokenString.compare("="))
      return TokenType::EqualOperator;
    else if (!tokenString.compare("!="))
      return TokenType::UnequalOperator;
    else if (!tokenString.compare(">"))
      return TokenType::GreaterOperator;
    else if (!tokenString.compare("<"))
      return TokenType::SmallerOperator;
    else if (!tokenString.compare(">="))
      return TokenType::GreaterEqualOperator;
    else if (!tokenString.compare("<="))
      return TokenType::SmallerEqualOperator;
    else if (!tokenString.compare("(")
             || !tokenString.compare(")")
             || !tokenString.compare(","))
      return TokenType::Delimiter;
    else
      return TokenType::Invalid;
  }
}
