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

    Lexer lexer;
    std::vector<Token> tokens = lexer.tokenize("SELECT ColumnName WHERE 1 == 1");
    for (int index = 0; index < tokens.size(); index++) {
        logger->log(tablog::DEBUG, "L: " + tokens[index].getLexeme() + " T: " + std::to_string(tokens[index].getType()));
    }
    
    return 0;
}
