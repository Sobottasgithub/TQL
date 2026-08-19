#ifndef LEXER_H
#define LEXER_H

#include "token.h"

#include <tablog.h>

#include <vector>
#include <string>
#include <memory>

namespace tql {
  class Lexer {
    public:
      Lexer();
      void tokenize(const std::string query);
      
      Token peek();
      Token next();

      int getTokenCollectionSize();

    private:
      std::shared_ptr<tablog::Tablog> logger;

      std::vector<Token> tokens;

       TokenType getTokenStringAsType(std::string tokenString);
  };
}

#endif
