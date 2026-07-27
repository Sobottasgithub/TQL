#include <tablog.h>

#include "lexer.h"
#include "token.h"

#include <iostream>
#include <string>
#include <memory>

using namespace tql;

int main() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("TQL-test", true);

    std::string query;
    std::cout << "Please enter a query: ";
    std::getline(std::cin, query);
        
    Lexer lexer;
    std::vector<Token> tokens = lexer.tokenize(query);
    for (int index = 0; index < tokens.size(); index++) {
        logger->log(tablog::DEBUG, "L: " + tokens[index].getLexeme() + " T: " + tokens[index].getTypeAsString());
    }
    
    return 0;
}
