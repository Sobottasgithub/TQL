#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "lexer.h"

#include <cstddef>
#include <tablog.h>

#include <vector>
#include <tuple>
#include <memory>

namespace tql {
  class Parser {
    public:
      struct Expression {
        Token token;
        std::vector<Expression> expressions;
      };
      
      Parser();
      Parser::Expression parse(Lexer lexer);
      
    private:
      std::shared_ptr<tablog::Tablog> logger;

      Expression parseTokens(Lexer* lexer);
      Expression parseSelect(Lexer* lexer);
      Expression parseColumns(Lexer* lexer);
      Expression parseAtom(Lexer* lexer);
      Expression parseAs(Lexer* lexer);
      Expression parseCount(Lexer* lexer);
      Expression parseMinMax(Lexer* lexer, TokenType aggregateTokenType);
      Expression parseMax(Lexer* lexer);
      Expression parseMin(Lexer* lexer);
      Expression parseFrom(Lexer* lexer);
  };
}

#endif
