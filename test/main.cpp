#include <tablog.h>

#include "lexer.h"
#include "token.h"
#include "parser.h"

#include <iostream>
#include <string>
#include <memory>

using namespace tql;
void displayExpressionTree(Parser::Expression expression) {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("ExpressionTree", true);

    logger->log(tablog::DEBUG, "First Expression: " + expression.token.getLexeme());
}

int main() {
    std::string query;
    std::cout << "Please enter a query: ";
    std::getline(std::cin, query);
        
    Lexer lexer;
    std::vector<Token> tokens = lexer.tokenize(query);
    Parser parser;
    Parser::Expression expression = parser.parse(tokens);
    displayExpressionTree(expression);
    
    return 0;
}
