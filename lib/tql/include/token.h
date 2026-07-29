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

      inline static std::vector<std::string> operators = {"WHERE", "AND", "OR", "NOT",
                                                          "=", "<", ">", "IN", "BETWEEN", "ASC",
                                                          "DESC", "ORDER", "BY"};
      inline static std::vector<std::string> delimiters = {",", "(", ")"};
                                 
      enum TokenType getType();
      std::string getLexeme();

      std::string getTypeAsString();

    private:
      std::string lexeme;
      enum TokenType type;
  };
}

#endif
