#ifndef TOKEN_H
#define TOKEN_H

#include "token_type.h"

#include <string>
#include <vector>

namespace tql {
  class Token {
    public:
      Token() {};
      Token(std::string lexeme, enum TokenType type);
                                 
      enum TokenType getType() const;
      std::string getLexeme();

      std::string getTypeAsString();

    private:
      std::string lexeme;
      enum TokenType type;
  };
}

#endif
