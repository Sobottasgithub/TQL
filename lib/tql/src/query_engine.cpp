#include "../include/query_engine.h"

#include "../include/token.h"
#include "../include/token_type.h"
#include "../include/parser.h"
#include "../include/interpreter.h"
#include "../include/execution_endpoint.h"

#include <arrow/type.h>
#include <string>
#include <tablog.h>
#include <vector>
#include <memory>
#include <arrow/table.h>

namespace tql {
  QueryEngine::QueryEngine() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("QueryEngine", true);
    this->logger = logger;

    this->interpreter.setOpenFile([this](std::string filePath) {
        return this->executionEndpoint.openFile(filePath);
    });

    this->interpreter.setGetWhere([this](std::string operatorName, std::string columnName, std::string compareValue, std::shared_ptr<arrow::Table> table) {
        return this->executionEndpoint.getWhere(operatorName, columnName, compareValue, table);
    });

    this->interpreter.setSelectColumns([this](std::vector<std::string> columnNames, std::shared_ptr<arrow::Table> table) {
        return this->executionEndpoint.selectColumns(columnNames, table);
    });

    this->interpreter.setGetDistinct([this](std::shared_ptr<arrow::Table> table) {
        return this->executionEndpoint.getDistinct(table);
    });

    this->interpreter.setGetCount([this](std::shared_ptr<arrow::Table> table) {
        return this->executionEndpoint.getCount(table);
    });

    this->interpreter.setGetMin([this](std::shared_ptr<arrow::Table> table) {
        return this->executionEndpoint.getMin(table);
    });

    this->interpreter.setGetMax([this](std::shared_ptr<arrow::Table> table) {
        return this->executionEndpoint.getMax(table);
    });

    this->interpreter.setGetRenamedTable([this](std::string originalColumnName, std::string newColumnName, std::shared_ptr<arrow::Table> table) {
        return this->executionEndpoint.getRenamedTable(originalColumnName, newColumnName, table);
    });
  }

  std::shared_ptr<arrow::Table> QueryEngine::execute(std::string query) {
    Lexer lexer;
    lexer.tokenize(query);

    // DEBUG: Display lexer output:
    // while (lexer.peek().getType() != TokenType::Eof) {
    //     Token currentToken = lexer.next();
    //     this->logger->log(tablog::DEBUG, currentToken.getTypeAsString() + " >> " + currentToken.getLexeme());
    // }

    Parser::Expression expressionTree = this->parser.parse(lexer);

    // DEBUG: Display expression Tree:
    displayExpressionTree(expressionTree);

    return this->interpreter.interpret(expressionTree);

    // DEBUG
    // return nullptr;
  }

  void QueryEngine::displayExpressionTree(Parser::Expression expression) {
      for (int index = 0; index < expression.expressions.size(); index++) {
          displayExpressionTree(expression.expressions[index]);
      }
 
      if (expression.token.getType() == TokenType::Atom)
          this->logger->log(tablog::DEBUG, expression.token.getTypeAsString() + " -> " + expression.token.getLexeme());
      else
          this->logger->log(tablog::DEBUG, expression.token.getTypeAsString());
  }
}
