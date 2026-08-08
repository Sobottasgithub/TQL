#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

namespace tql {
  enum TokenType {
    Atom,
    Columns,
    All,
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
