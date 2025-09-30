#pragma once

#include "filter_structs.h"
#include <functional>
#include <unordered_map>

/**
 * The language takes a FilterCondition structure and converts it into a lambda function
 * That can be applied to a vector of Key objects to evaluate the condition.
 */

class LanguageParser {
public:
    // Parses a FilterCondition and returns a lambda function that evaluates it
    static std::function<bool(const std::vector<Key>&)> parse(const FilterCondition& condition);
private:
    // Helper functions to evaluate expressions
    static ValueType evaluateArithmetic(const ValueType& left, ArithmeticOperations op, const ValueType& right);
    static bool evaluateComparison(const ValueType& left, ComparisonOperations op, const ValueType& right);
    static ValueType getValueFromKey(const std::vector<Key>& keys, const std::string& keyName);
    static const ValueType* getValueFromKeyMap(const std::unordered_map<std::string, const ValueType*>& keyMap, const std::string& keyName);
};

class ParseException : public std::exception {
public:
    ParseException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
private:
    std::string message_;
};