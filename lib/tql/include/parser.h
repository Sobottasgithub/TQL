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
      Expression parse(Lexer lexer);
      
    private:
      std::shared_ptr<tablog::Tablog> logger;

      enum States {
        Start,
        AfterSelect,
        All,
        Column,
        From,
        AfterFrom
        
      };
  };
}

#endif
