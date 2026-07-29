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

  std::string Token::getTypeAsString() {
    switch(this->type) {
      case TokenType::Atom:
        return "Atom";
      case TokenType::SelectOperator:
        return "SelectOperator";
      case TokenType::CountOperator:
        return "CountOperator";
      case TokenType::DistinctOperator:
        return "DistinctOperator";
      case TokenType::AsOperator:
        return "AsOperator";
      case TokenType::FromOperator:
        return "From";
      case TokenType::Operator:
        return "Operator";
      case TokenType::Eof:
        return "Eof";
    }
  }
}
