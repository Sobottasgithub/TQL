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
    AsOperator,

    // Aggregate Functions
    CountOperator,
    MinOperator,
    MaxOperator,
    SumOperator,
    AvgOperator,
    
    FromOperator,

    // Where
    WhereOperator,
    EqualOperator,
    GreaterOperator,
    SmallerOperator,
    UnequalOperator,
    GreaterEqualOperator,
    SmallerEqualOperator,
    
    Operator,
    Eof,
    Invalid
  }; 
}

#endif
