#include "parser.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <functional>

std::function<bool(const std::vector<Key>&)> LanguageParser::parse(const FilterCondition& condition) {
    return [condition](const std::vector<Key>& keys) -> bool {
        bool result = true; // Default to true for AND operations
        for (const auto& subExpr : condition.sub_expressions) {
            bool subResult = false;
            if (std::holds_alternative<UnaryExpression>(subExpr.expr)) {
                const auto& expr = std::get<UnaryExpression>(subExpr.expr);
                ValueType keyValue = getValueFromKey(keys, expr.key);
                subResult = evaluateComparison(keyValue, expr.op, expr.value);
            } else if (std::holds_alternative<BinaryExpression>(subExpr.expr)) {
                const auto& expr = std::get<BinaryExpression>(subExpr.expr);
                ValueType leftValue = getValueFromKey(keys, expr.left_key);
                ValueType rightValue = getValueFromKey(keys, expr.right_key);
                ValueType arithResult = evaluateArithmetic(leftValue, expr.arith_op, rightValue);
                subResult = evaluateComparison(arithResult, expr.comp_op, expr.value);
            } else {
                throw ParseException("Unknown expression type");
            }

            // Combine with previous results using the logical operator
            switch (subExpr.prev_logical_op) {
                case LogicalOperations::AND:
                    result = result && subResult;
                    break;
                case LogicalOperations::OR:
                    result = result || subResult;
                    break;
                case LogicalOperations::NONE:
                    result = subResult; // For the first expression
                    break;
                default:
                    throw ParseException("Unsupported logical operation");
            }
        }
        return result;
    };
}

ValueType LanguageParser::evaluateArithmetic(const ValueType& left, ArithmeticOperations op, const ValueType& right) {
    if (std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right)) {
        int64_t l = std::get<int64_t>(left);
        int64_t r = std::get<int64_t>(right);
        switch (op) {
            case ArithmeticOperations::ADD: return l + r;
            case ArithmeticOperations::SUBTRACT: return l - r;
            case ArithmeticOperations::MULTIPLY: return l * r;
            case ArithmeticOperations::DIVIDE:
                if (r == 0) throw ParseException("Division by zero");
                return l / r;
            default: throw ParseException("Unsupported arithmetic operation");
        }
    } else if ((std::holds_alternative<int64_t>(left) || std::holds_alternative<double>(left)) &&
               (std::holds_alternative<int64_t>(right) || std::holds_alternative<double>(right))) {
        double l = std::holds_alternative<int64_t>(left) ? static_cast<double>(std::get<int64_t>(left)) : std::get<double>(left);
        double r = std::holds_alternative<int64_t>(right) ? static_cast<double>(std::get<int64_t>(right)) : std::get<double>(right);
        switch (op) {
            case ArithmeticOperations::ADD: return l + r;
            case ArithmeticOperations::SUBTRACT: return l - r;
            case ArithmeticOperations::MULTIPLY: return l * r;
            case ArithmeticOperations::DIVIDE:
                if (r == 0.0) throw ParseException("Division by zero");
                return l / r;
            default: throw ParseException("Unsupported arithmetic operation");
        }
    } else {
        throw ParseException("Arithmetic operations require numeric types");
    }
}

bool LanguageParser::evaluateComparison(const ValueType& left, ComparisonOperations op, const ValueType& right) {
    if (left.index() != right.index()) {
        throw ParseException("Comparison requires operands of the same type");
    }

    if (std::holds_alternative<int64_t>(left)) {
        int64_t l = std::get<int64_t>(left);
        int64_t r = std::get<int64_t>(right);
        switch (op) {
            case ComparisonOperations::EQUAL: return l == r;
            case ComparisonOperations::NOT_EQUAL: return l != r;
            case ComparisonOperations::GREATER_THAN: return l > r;
            case ComparisonOperations::LESS_THAN: return l < r;
            case ComparisonOperations::GREATER_EQUAL: return l >= r;
            case ComparisonOperations::LESS_EQUAL: return l <= r;
            default: throw ParseException("Unsupported comparison operation");
        }
    } else if (std::holds_alternative<double>(left)) {
        double l = std::get<double>(left);
        double r = std::get<double>(right);
        switch (op) {
            case ComparisonOperations::EQUAL: return l == r;
            case ComparisonOperations::NOT_EQUAL: return l != r;
            case ComparisonOperations::GREATER_THAN: return l > r;
            case ComparisonOperations::LESS_THAN: return l < r;
            case ComparisonOperations::GREATER_EQUAL: return l >= r;
            case ComparisonOperations::LESS_EQUAL: return l <= r;
            default: throw ParseException("Unsupported comparison operation");
        }
    } else if (std::holds_alternative<std::string>(left)) {
        const std::string& l = std::get<std::string>(left);
        const std::string& r = std::get<std::string>(right);
        switch (op) {
            case ComparisonOperations::EQUAL: return l == r;
            case ComparisonOperations::NOT_EQUAL: return l != r;
            case ComparisonOperations::GREATER_THAN: return l > r;
            case ComparisonOperations::LESS_THAN: return l < r;
            case ComparisonOperations::GREATER_EQUAL: return l >= r;
            case ComparisonOperations::LESS_EQUAL: return l <= r;
            default: throw ParseException("Unsupported comparison operation");
        }
    } else if (std::holds_alternative<bool>(left)) {
        bool l = std::get<bool>(left);
        bool r = std::get<bool>(right);
        switch (op) {
            case ComparisonOperations::EQUAL: return l == r;
            case ComparisonOperations::NOT_EQUAL: return l != r;
            default: throw ParseException("Unsupported comparison operation for boolean");
        }
    } else {
        throw ParseException("Unsupported type for comparison");
    }
}

ValueType LanguageParser::getValueFromKey(const std::vector<Key>& keys, const std::string& keyName) {
    auto it = std::find_if(keys.begin(), keys.end(), [&keyName](const Key& k) { return k.getName() == keyName; });
    if (it != keys.end()) {
        return it->getValue();
    } else {
        throw ParseException("Key not found: " + keyName);
    }
}

bool LanguageParser::evaluateLogical(bool left, LogicalOperations op, bool right) {
    switch (op) {
        case LogicalOperations::AND: return left && right;
        case LogicalOperations::OR: return left || right;
        default: throw ParseException("Unsupported logical operation");
    }
}

