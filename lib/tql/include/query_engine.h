#ifndef QUERY_ENGINE_H
#define QUERY_ENGINE_H

#include <string>
#include <memory>
#include <arrow/table.h>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "execution_endpoint.h"

namespace tql {
  class QueryEngine {
    public:
      QueryEngine();

      std::shared_ptr<arrow::Table> execute(std::string query);

      void displayExpressionTree(Parser::Expression expression);
      
    private:
      std::shared_ptr<tablog::Tablog> logger;

      ExecutionEndpoint executionEndpoint;
      Parser parser;
      Interpreter interpreter;
  };
}

#endif
