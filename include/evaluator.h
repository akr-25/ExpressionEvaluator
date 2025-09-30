#pragma once
#include "parser.h"
#include <functional>

class Evaluator {
  public: 
    void initialize(const FilterCondition& condition) {
        evaluator_ = LanguageParser::parse(condition);
    }
    bool evaluate(const std::vector<Key>& keys) {
        return evaluator_(keys);
    }
  private:
    std::function<bool(const std::vector<Key>&)> evaluator_;  
};