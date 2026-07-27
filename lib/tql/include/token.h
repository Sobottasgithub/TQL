#ifndef TOKEN_H
#define TOKEN_H

#include "token_type.h"

#include <string>

namespace tql {
  class Token {
    public:
      Token(std::string lexeme, enum TokenType type);
      enum TokenType getType();
      std::string getLexeme();

    private:
      std::string lexeme;
      enum TokenType type;
  };
}

#endif
