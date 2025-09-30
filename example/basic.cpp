#include "evaluator.h"
#include <iostream>

int main() {
    // Example usage of the Evaluator with a simple filter condition
    FilterCondition condition = {
        {
            { UnaryExpression{ComparisonOperations::GREATER_THAN, "A", int64_t(10)}, LogicalOperations::NONE },
            { UnaryExpression{ComparisonOperations::LESS_THAN, "B", int64_t(20)}, LogicalOperations::AND }
        }
    };

    Evaluator evaluator;
    evaluator.initialize(condition);

    std::vector<Key> keys = {
        Key("A", int64_t(15)),
        Key("B", int64_t(15))
    };

    bool result = evaluator.evaluate(keys);
    std::cout << "Evaluation result: " << (result ? "true" : "false") << std::endl;

    return 0;
}