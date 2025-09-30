#pragma once
#include <string>
#include <variant>
#include <vector>

using ValueType = std::variant<int64_t, double, std::string, bool>;

class Key {
public:
  Key(const std::string &name, const ValueType &value)
      : name_(name), value_(value) {}

  const std::string &getName() const { return name_; }
  const ValueType &getValue() const { return value_; }
  void setValue(const ValueType &value) { value_ = value; }

private:
  std::string name_;
  ValueType value_;
};
