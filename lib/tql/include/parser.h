#ifndef PARSER_H
#define PARSER_H

#include "token.h"

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
      Parser::Expression parse(std::vector<Token> tokens);
      
    private:
      std::shared_ptr<tablog::Tablog> logger;

      Expression parseTokens(std::vector<Token> tokens, int cursor);
      std::tuple<Expression, int> parseSelect(std::vector<Token> tokens, int cursor);
      std::tuple<Expression, int> parseColumns(std::vector<Token> tokens, int cursor);
      std::tuple<Expression, int> parseDistinct(std::vector<Token> tokens, int cursor);
      std::tuple<Expression, int> parseCount(std::vector<Token> tokens, int cursor);
      std::tuple<Expression, int> parseFrom(std::vector<Token> tokens, int cursor);
  };
}

#endif
