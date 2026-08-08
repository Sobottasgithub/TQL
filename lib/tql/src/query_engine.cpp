#include "../include/query_engine.h"

#include "../include/token.h"
#include "../include/token_type.h"
#include "../include/parser.h"
#include "../include/interpreter.h"
#include "../include/execution_endpoint.h"

#include <string>
#include <vector>
#include <memory>
#include <arrow/table.h>

namespace tql {
  QueryEngine::QueryEngine() {
    ExecutionEndpoint executionEndpoint;
      
    this->interpreter.setOpenFile([&executionEndpoint](std::string filePath) {
        return executionEndpoint.openFile(filePath);
    });

    this->interpreter.setSelectColumns([&executionEndpoint](std::vector<std::string> columnNames, std::shared_ptr<arrow::Table> table) {
        return executionEndpoint.selectColumns(columnNames, table);
    });
  }

  std::shared_ptr<arrow::Table> QueryEngine::execute(std::string query) {
    Lexer lexer;
    lexer.tokenize(query);

    // DEBUG: Display lexer output:
    // for (int index = 0; index < tokens.size(); index++) {
    //     std::cout << tokens[index].getLexeme() << " >> " << tokens[index].getTypeAsString() << std::endl;      
    // }

    Parser::Expression expressionTree = this->parser.parse(lexer);

    // DEBUG: Display expression Tree:
    displayExpressionTree(expressionTree);

    return this->interpreter.interpret(expressionTree);
  }

  void QueryEngine::displayExpressionTree(Parser::Expression expression) {
      std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
      logger->configure("ExpressionTree", true);

      // Post order
      if (expression.token.getType() == TokenType::Columns) {
          for (int index = 0; index < expression.expressions.size(); index++) {
              displayExpressionTree(expression.expressions[index]);
          }
      } else {
          if (expression.expressions.size() >= 2) // Right side
              displayExpressionTree(expression.expressions[1]);
          
          if (expression.expressions.size() >= 1) // Left side
              displayExpressionTree(expression.expressions[0]);
      }

      if (expression.token.getType() == TokenType::Atom)
          logger->log(tablog::DEBUG, expression.token.getTypeAsString() + " -> " + expression.token.getLexeme());
      else
          logger->log(tablog::DEBUG, expression.token.getTypeAsString());
  }
}
