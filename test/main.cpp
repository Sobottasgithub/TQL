#include <tablog.h>

#include "lexer.h"
#include "token.h"
#include "parser.h"
#include "token_type.h"

#include <iostream>
#include <string>
#include <memory>

using namespace tql;
void displayExpressionTree(Parser::Expression expression) {
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
