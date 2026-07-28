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
      std::vector<Token> tokenize(const std::string query);

    private:
      std::shared_ptr<tablog::Tablog> logger;

      bool tokenIsInVector(std::vector<std::string> vector, std::string tokenString);
  };
}

#endif
