#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"

#include <tablog.h>
#include <memory>
#include <optional>
#include <functional>
#include <arrow/table.h>
#include <string>

namespace tql {
  class Interpreter {
    public:
      using OpenFile = std::function<std::shared_ptr<arrow::Table>(std::string)>;
      
      Interpreter();

      void setOpenFile(OpenFile openFile);
      
      std::shared_ptr<arrow::Table> interpret(Parser::Expression expressionTree);
      
    private:
      std::shared_ptr<tablog::Tablog> logger;

      OpenFile openFile;

      std::shared_ptr<arrow::Table> interpretSelect(Parser::Expression expression);
      std::shared_ptr<arrow::Table> interpretFrom(Parser::Expression expression);

      std::optional<Parser::Expression> getExpressionByTokenType(TokenType requestedTokenType, std::vector<Parser::Expression> expressions);
  };
}

#endif
