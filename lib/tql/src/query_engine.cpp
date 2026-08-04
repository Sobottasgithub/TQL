#include "../include/query_engine.h"

#include "../include/token.h"
#include "../include/token_type.h"
#include "../include/parser.h"

#include <string>
#include <vector>

namespace tql {
  QueryEngine::QueryEngine() {
    
  }

  void QueryEngine::execute(std::string query) {
    std::vector<Token> tokens = this->lexer.tokenize(query);

    // DEBUG: Display lexer output:
    // for (int index = 0; index < tokens.size(); index++) {
    //     std::cout << tokens[index].getLexeme() << " >> " << tokens[index].getTypeAsString() << std::endl;      
    // }

    Parser::Expression expression = this->parser.parse(tokens);

    // DEBUG: Display expression Tree:
    displayExpressionTree(expression);
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
          if (expression.expressions.size() >= 1) // Left side
              displayExpressionTree(expression.expressions[0]);

          if (expression.expressions.size() >= 2) // Right side
              displayExpressionTree(expression.expressions[1]);
      }

      if (expression.token.getType() == TokenType::Atom)
          logger->log(tablog::DEBUG, expression.token.getTypeAsString() + " -> " + expression.token.getLexeme());
      else
          logger->log(tablog::DEBUG, expression.token.getTypeAsString());
  }
}
