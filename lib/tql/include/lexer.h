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

    private:
      std::shared_ptr<tablog::Tablog> logger;

      std::vector<Token> tokens;

      bool tokenIsInVector(std::vector<std::string> vector, std::string tokenString);
  };
}

#endif
