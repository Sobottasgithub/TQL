#include "../include/interpreter.h"

#include <tablog.h>
#include <memory>

namespace tql {
  Interpreter::Interpreter() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("Interpreter", true);
    this->logger = logger;
  }

  void Interpreter::interpret(Parser::Expression expressionTree) {
    
  }
}
