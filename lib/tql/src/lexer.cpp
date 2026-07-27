#include "../include/lexer.h"

#include <tablog.h>

#include <iostream>
#include <memory>

namespace tql {
  Lexer::Lexer() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("Lexer", true);

    logger->log(tablog::DEBUG, "Hello from Lexer");
  }
}
