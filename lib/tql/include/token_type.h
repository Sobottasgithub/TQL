#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

namespace tql {
  enum TokenType {
    Atom,
    DmlOperator,
    CardinalitiesOperator,
    FromOperator,
    Operator,
    Eof
  }; 
}

#endif
