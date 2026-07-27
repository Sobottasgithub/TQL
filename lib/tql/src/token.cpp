#include "../include/token.h"

namespace tql {
  Token::Token (std::string lexeme, enum TokenType type) {
    this->lexeme = lexeme;
    this->type = type;
  }

  enum TokenType Token::getType() {
    return this->type;
  }

  std::string Token::getLexeme() {
    return this->lexeme;
  }
}
