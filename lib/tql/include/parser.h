#ifndef PARSER_H
#define PARSER_H

#include "token.h"

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
      Expression parseTokens(std::vector<Token> tokens);
      
    private:
      std::shared_ptr<tablog::Tablog> logger;
      
      std::tuple<float, float> getOperatorWeights(std::string operatorString);
  };
}

#endif
