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
        AfterSelect,
        All,
        Column,
        AfterColumn,
        As,
        AfterAs,
        Distinct,
        Aggregate,
        AggregateDistinct,
        AfterAggregate,
        From,
        AfterFrom,
        Where,
        AfterWhereInfixAtom,
        AfterWhereInfixOperator,
        Invalid
      };

      Expression parseSelect(Lexer* lexer);
  };
}

#endif
