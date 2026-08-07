#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"

#include <tablog.h>
#include <memory>

namespace tql {
  class Interpreter {
    public:
      Interpreter();
      void interpret(Parser::Expression expressionTree);
      
    private:
      std::shared_ptr<tablog::Tablog> logger;
  };
}

#endif
