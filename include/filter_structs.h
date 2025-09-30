#pragma once

#include <string>
#include <vector>
#include <variant>

#include "enums.h"
#include "key.h"

// A filter condition structure is like (subexpression operator subexpression ...)
// for example (A + B < C) AND (A1 < C2) OR (A2 > C3)
// here As and Bs are keys and Cs are constants

struct UnaryExpression {
  ComparisonOperations op;
  std::string key;
  ValueType value;
};

struct BinaryExpression {
  std::string left_key;
  ArithmeticOperations arith_op; // +, -, *, /
  std::string right_key; // could be a key or a constant
  ComparisonOperations comp_op; // ==, !=, >, <, >=, <=
  ValueType value; // could be a key or a constant
};

struct SubExpression {
  std::variant<UnaryExpression, BinaryExpression> expr;
  LogicalOperations prev_logical_op; // AND, OR
};

struct FilterCondition {
  std::vector<SubExpression> sub_expressions;
};