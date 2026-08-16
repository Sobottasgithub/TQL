#include "../include/parser.h"

#include "../include/token.h"
#include "../include/token_type.h"
#include "../include/lexer.h"

#include <tablog.h>

#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace tql {
  Parser::Parser() {
    std::shared_ptr<tablog::Tablog> logger = std::make_shared<tablog::Tablog>();
    logger->configure("Parser", true);
    this->logger = logger;
  }

  Parser::Expression Parser::parse(Lexer lexer) {
    Expression parentExpression;
    if (lexer.peek().getType() == TokenType::SelectOperator) {
      parentExpression.token = lexer.next();

      while (lexer.peek().getType() != TokenType::Eof || lexer.peek().getType() == TokenType::Delimiter) {
        parseRecursiv(&parentExpression, &lexer);
      }

      // INFO: Bundle atoms in columnsExpression
      Parser::Expression columnsExpression;
      columnsExpression.token = Token("", TokenType::Columns);
      // Copy atoms to columnsExpression
      std::vector<Parser::Expression>* selectChildExpressions = &parentExpression.expressions;
      std::copy_if(selectChildExpressions->begin(), selectChildExpressions->end(), std::back_inserter(columnsExpression.expressions),
        [](const Parser::Expression& expression) { return expression.token.getType() == TokenType::Atom; }
      );
      // Remove atoms from selecTExpression
      selectChildExpressions->erase(
        std::remove_if(selectChildExpressions->begin(), selectChildExpressions->end(),
            [](const Parser::Expression& expression) { return expression.token.getType() == TokenType::Atom; }),
        selectChildExpressions->end()
      );
      if (columnsExpression.expressions.size() > 0)
        parentExpression.expressions.push_back(columnsExpression);

      return parentExpression;
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected token of type Dml!");
      throw "Bad Token";
    }
  }

  void Parser::parseRecursiv(Parser::Expression* parentExpression, Lexer* lexer) {
    // SELECT
    if (parentExpression->token.getType() == TokenType::FromOperator
        && lexer->peek().getType() == TokenType::SelectOperator) {
      Parser::Expression selectExpression;
      selectExpression.token = lexer->next();

      while (lexer->peek().getType() != TokenType::Eof || lexer->peek().getType() == TokenType::Delimiter) {
        parseRecursiv(&selectExpression, lexer);
      }

      // INFO: Bundle atoms in columnsExpression
      Parser::Expression columnsExpression;
      columnsExpression.token = Token("", TokenType::Columns);
      // Copy atoms to columnsExpression
      std::vector<Parser::Expression>* selectChildExpressions = &selectExpression.expressions;
      std::copy_if(selectChildExpressions->begin(), selectChildExpressions->end(), std::back_inserter(columnsExpression.expressions),
        [](const Parser::Expression& expression) { return expression.token.getType() == TokenType::Atom; }
      );
      // Remove atoms from selecTExpression
      selectChildExpressions->erase(
        std::remove_if(selectChildExpressions->begin(), selectChildExpressions->end(),
            [](const Parser::Expression& expression) { return expression.token.getType() == TokenType::Atom; }),
        selectChildExpressions->end()
      );
      selectExpression.expressions.push_back(columnsExpression);
      
      parentExpression->expressions.push_back(selectExpression);
      return;
    }

    // DISTINCT
    if (std::find(this->distinctParentTypes.begin(), this->distinctParentTypes.end(), parentExpression->token.getType()) != this->distinctParentTypes.end()
        && lexer->peek().getType() == TokenType::DistinctOperator) {
      Parser::Expression distinctExpression;
      distinctExpression.token = lexer->next();
      parentExpression->expressions.push_back(distinctExpression);

      // WARNING: Passing the parentExpression to the parseRecursiv function is intentional here
      // The DistinctOperator has no children itself
      parseRecursiv(parentExpression, lexer);
      return;
    }

    // FROM
    if (parentExpression->token.getType() == TokenType::SelectOperator && lexer->peek().getType() == TokenType::FromOperator) {
      Parser::Expression fromExpression;
      fromExpression.token = lexer->next();
      
      parseRecursiv(&fromExpression, lexer);
      parentExpression->expressions.push_back(fromExpression);
      return;
    }

    // ATOM
    if (std::find(this->atomParentTypes.begin(), this->atomParentTypes.end(), parentExpression->token.getType()) != this->atomParentTypes.end()
        && lexer->peek().getType() == TokenType::Atom) {
      Parser::Expression atomExpression;
      atomExpression.token = lexer->next();

      if (lexer->peek().getType() == TokenType::AsOperator) {
        parseRecursiv(&atomExpression, lexer);
      }

      parentExpression->expressions.push_back(atomExpression);
      return;
    }

    // ALL (*)
    if (std::find(this->atomParentTypes.begin(), this->atomParentTypes.end(), parentExpression->token.getType()) != this->atomParentTypes.end()
        && lexer->peek().getType() == TokenType::All) {
      Parser::Expression allExpression;
      allExpression.token = lexer->next();

      parentExpression->expressions.push_back(allExpression);
      return;
    }

    // AS
    if (parentExpression->token.getType() == TokenType::Atom && lexer->peek().getType() == TokenType::AsOperator) {
      Parser::Expression asExpression;
      asExpression.token = lexer->next();

      parseRecursiv(&asExpression, lexer);

      parentExpression->expressions.push_back(asExpression);
      return;
    }

    // EOF
    if (lexer->peek().getType() == TokenType::Eof) {
      Parser::Expression eofExpression;
      eofExpression.token = lexer->next();
      parentExpression->expressions.push_back(eofExpression);
      return;
    }

    // DELIMITER
    if (lexer->peek().getType() == TokenType::Delimiter) {
      if (lexer->peek().getLexeme() == "(") {
        lexer->next();
        parseRecursiv(parentExpression, lexer);
      } else {
        lexer->next();
      }

      return;
    }

    throw std::invalid_argument("Bad Token! " + lexer->peek().getTypeAsString());
  }

  Parser::Expression Parser::parseSelect(Lexer* lexer) {
    Parser::Expression selectExpression;
    if (lexer->peek().getType() == TokenType::SelectOperator) {
      selectExpression.token = lexer->next();
      
      for (Parser::Expression expression : parseAggregateFunctions(lexer)) {
        selectExpression.expressions.push_back(expression);
      }

      Parser::Expression fromResult = parseFrom(lexer);
      selectExpression.expressions.push_back(fromResult);

      return selectExpression;
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected token of type SELECT!");
      throw "Bad Token";
    }
    return selectExpression;
  }

  Parser::Expression Parser::parseColumns(Lexer* lexer) {
    Expression columnsExpression;
    columnsExpression.token = Token("", TokenType::Columns);

    do {
      if (lexer->peek().getLexeme().compare(",") == 0)
        lexer->next();
      
      if (lexer->peek().getType() == TokenType::Atom) {
        Parser::Expression parsedAtom = parseAtom(lexer);
        columnsExpression.expressions.push_back(parsedAtom);
      }
    } while (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme().compare(",") == 0);
    
    return columnsExpression;    
  }

  Parser::Expression Parser::parseAtom(Lexer* lexer) {
    Expression atomExpression;

    if (lexer->peek().getType() == TokenType::Atom) {
        atomExpression.token = lexer->next();
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected Atom!");
      throw "Bad Token";
    }

    if (lexer->peek().getType() == TokenType::AsOperator) {
      Parser::Expression parsedAs = parseAs(lexer);
      atomExpression.expressions.push_back(parsedAs);
    }

    return atomExpression;
  }

  Parser::Expression Parser::parseAll(Lexer* lexer) {
    Expression allExpression;
    allExpression.token = lexer->next();
    return allExpression;
  }

  Parser::Expression Parser::parseAs(Lexer* lexer) {
    Expression asExpression;

    if (lexer->peek().getType() == TokenType::AsOperator) {
      asExpression.token = lexer->next();
      
      Parser::Expression parsedAtom = parseAtom(lexer);
      asExpression.expressions.push_back(parsedAtom);
    }

    return asExpression;
  }
  
  Parser::Expression Parser::parseCount(Lexer* lexer) {
    Expression countExpression;

    if (lexer->peek().getType() == TokenType::CountOperator) {
      countExpression.token = lexer->next();

      // INFO: Delimiters are not added to parser tree
      if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme().compare("(") == 0) {
        lexer->next();
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected delimiter: '(' !");
        throw "Bad Token";
      }

      if (lexer->peek().getType() == TokenType::DistinctOperator) {
        Expression distinctExpression;
        distinctExpression.token = lexer->next();
        
        if (lexer->peek().getType() == TokenType::Atom) {
          Parser::Expression result = parseColumns(lexer);
          countExpression.expressions.push_back(result);
        } else {
          this->logger->log(tablog::ERROR, "Bad Token! Count Distinct expects column names!");
          throw "Bad Token";
        }
      } else if (lexer->peek().getType() == TokenType::All) {
        Parser::Expression allExpression = parseAll(lexer);
        countExpression.expressions.push_back(allExpression);
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Count expected * or distinct columns!");
        throw "Bad Token";
      }

      if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme().compare(")") == 0) {
        lexer->next();
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected delimiter: ')' !");
        throw "Bad Token";
      }

      if (lexer->peek().getType() == TokenType::AsOperator) {
        Parser::Expression parsedAs = parseAs(lexer);
        countExpression.expressions.push_back(parsedAs);
      }
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected Count!");
      throw "Bad Token";
    }

    return countExpression;
  }

  Parser::Expression Parser::parseMinMax(Lexer* lexer, TokenType aggregateTokenType) {
    Expression minMaxExpression;

    if (lexer->peek().getType() == aggregateTokenType) {
      minMaxExpression.token = lexer->next();

      // INFO: Delimiters are not added to parser tree
      if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme().compare("(") == 0) {
        lexer->next();
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected delimiter: '(' !");
        throw "Bad Token";
      }

      if (lexer->peek().getType() == TokenType::DistinctOperator) {
        Expression distinctExpression;
        distinctExpression.token = lexer->next();
        
        if (lexer->peek().getType() == TokenType::Atom) {
          Parser::Expression result = parseAtom(lexer);
          distinctExpression.expressions.push_back(result);
        } else {
          this->logger->log(tablog::ERROR, "Bad Token! Min Distinct expects a column name!");
          throw "Bad Token";
        }
        minMaxExpression.expressions.push_back(distinctExpression);
      } else if (lexer->peek().getType() == TokenType::Atom) {
        Parser::Expression result = parseAtom(lexer);
        minMaxExpression.expressions.push_back(result);
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Min/Max Distinct expects a column name!");
        throw "Bad Token";
      }
      
      if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme().compare(")") == 0) {
        lexer->next();
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected delimiter: ')' !");
        throw "Bad Token";
      }

      if (lexer->peek().getType() == TokenType::AsOperator) {
        Parser::Expression parsedAs = parseAs(lexer);
        minMaxExpression.expressions.push_back(parsedAs);
      }
    } else {
      this->logger->log(tablog::ERROR, "Bad Token! Expected Min/Max!");
      throw "Bad Token";
    }

    return minMaxExpression;
  }

  Parser::Expression Parser::parseMax(Lexer* lexer) {
    return parseMinMax(lexer, TokenType::MaxOperator);
  }

  Parser::Expression Parser::parseMin(Lexer* lexer) {
    return parseMinMax(lexer, TokenType::MinOperator);
  }

  std::vector<Parser::Expression> Parser::parseAggregateFunctions(Lexer* lexer) {
    std::vector<Parser::Expression> resultExpressionCollection = {};
    if (lexer->peek().getType() == TokenType::DistinctOperator) {
      Expression distinctExpression;
      distinctExpression.token = lexer->next();
      resultExpressionCollection.push_back(distinctExpression);
    
      if (lexer->peek().getType() == TokenType::Atom) {
        resultExpressionCollection.push_back(parseColumns(lexer));
      } else if (lexer->peek().getType() == TokenType::All) {
        resultExpressionCollection.push_back(parseAll(lexer));
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected atom or * after Distinct!");
        throw "Bad Token";
      }
    } else {
      int tokenType = lexer->peek().getType();
      if (tokenType == TokenType::Atom) {
        resultExpressionCollection.push_back(parseColumns(lexer));
      } else if (tokenType == TokenType::All) {
        resultExpressionCollection.push_back(parseAll(lexer));
      } else if (tokenType == TokenType::CountOperator) {
        resultExpressionCollection.push_back(parseCount(lexer));
      } else if (tokenType == TokenType::MinOperator) {
        resultExpressionCollection.push_back(parseMin(lexer));
      } else if (tokenType == TokenType::MaxOperator) {
        resultExpressionCollection.push_back(parseMax(lexer));
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected atom or count operator!");
        throw "Bad Token";
      }
    }
    return resultExpressionCollection;
  }
  
  Parser::Expression Parser::parseFrom(Lexer* lexer) {
    Expression fromExpression;

    if (lexer->peek().getType() == TokenType::FromOperator) {
      fromExpression.token = lexer->next();
      if (lexer->peek().getType() == TokenType::Atom) {
        Parser::Expression parsedAtom = parseAtom(lexer);
        fromExpression.expressions.push_back(parsedAtom);
      } else if (lexer->peek().getType() == TokenType::Delimiter && lexer->peek().getLexeme().compare("(") == 0) {
          lexer->next();
          if (lexer->peek().getType() == TokenType::SelectOperator) {
            Parser::Expression selectResult = parseSelect(lexer);
            fromExpression.expressions.push_back(selectResult);
            
            if (lexer->peek().getLexeme().compare(")") != 0) {
              this->logger->log(tablog::ERROR, "Bad Token! Expected closing Delimiter after Select operator in From operation!");
              throw "Bad Token";
            }

            lexer->next();
          } else {
            this->logger->log(tablog::ERROR, "Bad Token! Expected Select operator after Delimiter!");
            throw "Bad Token";
          }
      } else {
        this->logger->log(tablog::ERROR, "Bad Token! Expected Atom or Delimiter after From operator!");
        throw "Bad Token";
      }
    }
    return fromExpression;
  }
}
