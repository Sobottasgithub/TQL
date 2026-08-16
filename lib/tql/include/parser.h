#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "lexer.h"

#include <cstddef>
#include <tablog.h>

#include <vector>
#include <tuple>
#include <memory>

namespace tql {
  class Parser {
    public:
      struct Expression {
        Token token;
        std::vector<Expression> expressions;
      };
      
      Parser();
      Expression parse(Lexer lexer);
      
    private:
      std::shared_ptr<tablog::Tablog> logger;

      void parseRecursiv(Expression* parentExpression, Lexer* lexer);

      std::vector<TokenType> selectChildTypes = {TokenType::DistinctOperator, TokenType::Atom, TokenType::All, TokenType::FromOperator,
                                                 TokenType::CountOperator, TokenType::MinOperator, TokenType::MaxOperator, TokenType::Eof};
      std::vector<TokenType> distinctParentTypes = {TokenType::SelectOperator, TokenType::CountOperator, TokenType::MinOperator,
                                                    TokenType::MaxOperator, TokenType::AvgOperator, TokenType::SumOperator, TokenType::Columns};
      std::vector<TokenType> atomParentTypes = {TokenType::SelectOperator, TokenType::CountOperator, TokenType::AvgOperator, TokenType::SumOperator,
                                                TokenType::MaxOperator, TokenType::MinOperator, TokenType::AsOperator, TokenType::FromOperator};
      std::vector<TokenType> allParentTypes = {TokenType::SelectOperator, TokenType::MaxOperator, TokenType::MinOperator, TokenType::CountOperator,
                                               TokenType::AvgOperator, TokenType::SumOperator};
      
      Expression parseTokens(Lexer* lexer);
      Expression parseSelect(Lexer* lexer);
      Expression parseColumns(Lexer* lexer);
      Expression parseAtom(Lexer* lexer);
      Expression parseAll(Lexer* lexer);
      Expression parseAs(Lexer* lexer);
      Expression parseCount(Lexer* lexer);
      Expression parseMinMax(Lexer* lexer, TokenType aggregateTokenType);
      Expression parseMax(Lexer* lexer);
      Expression parseMin(Lexer* lexer);
      std::vector<Parser::Expression> parseAggregateFunctions(Lexer* lexer);
      Expression parseFrom(Lexer* lexer);
  };
}

#endif
