#pragma once
#include <string>
#include <variant>
#include <vector>
#include <utility>

using ValueType = std::variant<int64_t, double, std::string, bool>;

class Key {
public:
  Key(const std::string &name, const ValueType &value)
      : name_(name), value_(value) {}
  
  // Add move constructor for better performance
  Key(std::string &&name, ValueType &&value)
      : name_(std::move(name)), value_(std::move(value)) {}

  const std::string &getName() const { return name_; }
  const ValueType &getValue() const { return value_; }
  void setValue(const ValueType &value) { value_ = value; }
  void setValue(ValueType &&value) { value_ = std::move(value); }

private:
  std::string name_;
  ValueType value_;
};
