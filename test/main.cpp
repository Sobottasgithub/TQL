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

    // In order
    if (expression.expressions.size() >= 1)
        displayExpressionTree(expression.expressions[0]);
        
    logger->log(tablog::DEBUG, expression.token.getTypeAsString());

    if (expression.expressions.size() >= 2)
        displayExpressionTree(expression.expressions[1]);
}

int main() {
    std::string query;
    std::cout << "Please enter a query: ";
    std::getline(std::cin, query);

    Lexer lexer;
    std::vector<Token> tokens = lexer.tokenize(query);

    // for (int index = 0; index < tokens.size(); index++) {
    //     std::cout << tokens[index].getLexeme() << " >> " << tokens[index].getTypeAsString() << std::endl;      
    // }
    
    Parser parser;
    Parser::Expression expression = parser.parse(tokens);
    displayExpressionTree(expression);
    
    return 0;
}
