#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

namespace tql {
  enum TokenType {
    Atom,
    Columns,
    Delimiter,
    
    // DML operators
    SelectOperator,

    // CardinaltiesOperator
    DistinctOperator,
    CountOperator,
    AsOperator,
    
    FromOperator,
    Operator,
    Eof
  }; 
}

#endif
