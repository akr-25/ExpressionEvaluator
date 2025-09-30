#include "parser.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <functional>
#include <unordered_map>

std::function<bool(const std::vector<Key>&)> LanguageParser::parse(const FilterCondition& condition) {
    return [condition](const std::vector<Key>& keys) -> bool {
        // Build hash map for O(1) key lookups instead of O(n) linear search
        std::unordered_map<std::string, const ValueType*> keyMap;
        keyMap.reserve(keys.size());
        for (const auto& key : keys) {
            keyMap[key.getName()] = &key.getValue();
        }
        
        bool result = true; // Default to true for AND operations
        for (const auto& subExpr : condition.sub_expressions) {
            bool subResult = false;
            if (std::holds_alternative<UnaryExpression>(subExpr.expr)) {
                const auto& expr = std::get<UnaryExpression>(subExpr.expr);
                const ValueType* keyValue = getValueFromKeyMap(keyMap, expr.key);
                subResult = evaluateComparison(*keyValue, expr.op, expr.value);
            } else if (std::holds_alternative<BinaryExpression>(subExpr.expr)) {
                const auto& expr = std::get<BinaryExpression>(subExpr.expr);
                const ValueType* leftValue = getValueFromKeyMap(keyMap, expr.left_key);
                const ValueType* rightValue = getValueFromKeyMap(keyMap, expr.right_key);
                ValueType arithResult = evaluateArithmetic(*leftValue, expr.arith_op, *rightValue);
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

    // Template helper to reduce code duplication
    auto compare = [op](const auto& l, const auto& r) -> bool {
        using T = std::decay_t<decltype(l)>;
        switch (op) {
            case ComparisonOperations::EQUAL: return l == r;
            case ComparisonOperations::NOT_EQUAL: return l != r;
            case ComparisonOperations::GREATER_THAN: 
                if constexpr (std::is_same_v<T, bool>) {
                    throw ParseException("Unsupported comparison operation for boolean");
                }
                return l > r;
            case ComparisonOperations::LESS_THAN: 
                if constexpr (std::is_same_v<T, bool>) {
                    throw ParseException("Unsupported comparison operation for boolean");
                }
                return l < r;
            case ComparisonOperations::GREATER_EQUAL: 
                if constexpr (std::is_same_v<T, bool>) {
                    throw ParseException("Unsupported comparison operation for boolean");
                }
                return l >= r;
            case ComparisonOperations::LESS_EQUAL: 
                if constexpr (std::is_same_v<T, bool>) {
                    throw ParseException("Unsupported comparison operation for boolean");
                }
                return l <= r;
            default: throw ParseException("Unsupported comparison operation");
        }
    };

    if (std::holds_alternative<int64_t>(left)) {
        return compare(std::get<int64_t>(left), std::get<int64_t>(right));
    } else if (std::holds_alternative<double>(left)) {
        return compare(std::get<double>(left), std::get<double>(right));
    } else if (std::holds_alternative<std::string>(left)) {
        return compare(std::get<std::string>(left), std::get<std::string>(right));
    } else if (std::holds_alternative<bool>(left)) {
        return compare(std::get<bool>(left), std::get<bool>(right));
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

const ValueType* LanguageParser::getValueFromKeyMap(const std::unordered_map<std::string, const ValueType*>& keyMap, const std::string& keyName) {
    auto it = keyMap.find(keyName);
    if (it != keyMap.end()) {
        return it->second;
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

