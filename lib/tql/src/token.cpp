#include "../include/token.h"

namespace tql {  
  Token::Token (std::string lexeme, enum TokenType type) {
    this->lexeme = lexeme;
    this->type = type;
  }

  enum TokenType Token::getType() const {
    return this->type;
  }

  std::string Token::getLexeme() {
    return this->lexeme;
  }

  std::string Token::getTypeAsString() {
    switch(this->type) {
      case TokenType::Atom:
        return "Atom";
      case TokenType::Columns:
        return "Columns";
      case TokenType::All:
        return "*";
      case TokenType::Delimiter:
        return "Delimiter";
      case TokenType::SelectOperator:
        return "SelectOperator";
      case TokenType::CountOperator:
        return "CountOperator";
      case TokenType::MinOperator:
        return "MinOperator";
      case TokenType::MaxOperator:
        return "MaxOperator";
      case TokenType::SumOperator:
        return "SumOperator";
      case TokenType::AvgOperator:
        return "AvgOperator";
      case TokenType::DistinctOperator:
        return "DistinctOperator";
      case TokenType::AsOperator:
        return "AsOperator";
      case TokenType::FromOperator:
        return "From";
      case TokenType::WhereOperator:
        return "Where";
      case TokenType::EqualOperator:
        return "=";
      case TokenType::GreaterOperator:
        return ">";
      case TokenType::SmallerOperator:
        return "<";
      case TokenType::GreaterEqualOperator:
        return ">=";
      case TokenType::SmallerEqualOperator:
        return "<=";
      case TokenType::UnequalOperator:
        return "!=";
      case TokenType::Operator:
        return "Operator";
      case TokenType::Invalid:
        return "Invalid";
      case TokenType::Eof:
        return "Eof";
    }
  }
}
