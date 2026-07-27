#include "../include/lexer.h"

#include "../include/token.h"
#include "../include/token_type.h"

#include <tablog.h>

#include <iostream>
#include <memory>
#include <vector>

namespace tql {
  Lexer::Lexer() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("Lexer", true);
    this->logger = logger;
  }

  std::vector<Token> Lexer::tokenize(const std::string query) {
    std::vector<Token> tokens = {};
    Token token("Hello", TokenType::Atom);
    return tokens;
  }
}
