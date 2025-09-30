# runtime_language_parser

A high-performance C++17 runtime expression evaluator for complex filter conditions.

## Features

- Parse and evaluate complex filter conditions at runtime
- Support for multiple data types (int64_t, double, string, bool)
- Arithmetic operations (ADD, SUBTRACT, MULTIPLY, DIVIDE)
- Comparison operations (EQUAL, NOT_EQUAL, GREATER_THAN, LESS_THAN, GREATER_EQUAL, LESS_EQUAL)
- Logical operations (AND, OR)

## Performance Optimizations

This repository has been optimized for maximum performance:

### Key Optimizations Implemented

1. **Hash Map Key Lookup (O(1) vs O(n))**
   - Replaced linear search (`std::find_if`) with `std::unordered_map`
   - Significant performance improvement for expressions with many keys
   - Pre-allocation with `reserve()` to avoid rehashing

2. **Code Deduplication**
   - Template lambda functions in `evaluateComparison()` and `evaluateArithmetic()`
   - Reduced code size by ~40% in comparison logic
   - Better maintainability and compiler optimization opportunities

3. **Move Semantics**
   - Added move constructors to `Key` class
   - Move-enabled `setValue()` overload
   - Reduces unnecessary string and variant copies

4. **Build Optimizations**
   - Link-Time Optimization (LTO) enabled in Release builds
   - `-O3 -march=native -flto` compiler flags
   - Explicit file lists instead of `GLOB_RECURSE` for better incremental builds

5. **Dead Code Elimination**
   - Removed unused `evaluateLogical()` function
   - Cleaner codebase and smaller binary size

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Usage

```cpp
#include "evaluator.h"

// Create a filter condition
FilterCondition condition = {
    {
        { UnaryExpression{ComparisonOperations::GREATER_THAN, "A", int64_t(10)}, LogicalOperations::NONE },
        { UnaryExpression{ComparisonOperations::LESS_THAN, "B", int64_t(20)}, LogicalOperations::AND }
    }
};

// Initialize evaluator
Evaluator evaluator;
evaluator.initialize(condition);

// Create keys
std::vector<Key> keys = {
    Key("A", int64_t(15)),
    Key("B", int64_t(15))
};

// Evaluate
bool result = evaluator.evaluate(keys);  // Returns true
```

## Performance Characteristics

- Key lookup: O(1) average case
- Expression evaluation: O(n) where n is the number of sub-expressions
- Memory: O(k) where k is the number of keys

## Requirements

- C++17 compatible compiler
- CMake 3.10 or higher
