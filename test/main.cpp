#include <tablog.h>

#include "lexer.h"

#include <iostream>
#include <string>
#include <memory>

using namespace tql;

int main() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("TQL-test", true);

    logger->log(tablog::DEBUG, "Hello from TQL");

    Lexer lexer;
    
    return 0;
}
