#ifndef QUERY_ENGINE_H
#define QUERY_ENGINE_H

#include <string>

#include "lexer.h"
#include "parser.h"

namespace tql {
  class QueryEngine {
    public:
      QueryEngine();

      void execute(std::string query);

      void displayExpressionTree(Parser::Expression expression);
    private:
      Lexer lexer;
      Parser parser;
  };
}

#endif
