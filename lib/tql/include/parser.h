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

      Expression parseTokens(std::vector<Token> tokens, int cursor, float minWeight);
      std::tuple<float, float> getOperatorWeight(std::string operatorString);
  };
}

#endif
