# Expression Evaluator

A high-performance C++17 library for parsing and evaluating complex filter conditions at runtime. This library provides a flexible and type-safe way to evaluate expressions with arithmetic operations, comparisons, and logical operators on structured key-value data.

## Features

- **Type-Safe Expressions**: Supports multiple data types (int64_t, double, string, bool) with type checking
- **Arithmetic Operations**: Add, subtract, multiply, and divide operations
- **Comparison Operations**: Equality, inequality, greater than, less than, and their variants
- **Logical Operations**: AND, OR operations for combining multiple conditions
- **Flexible API**: Easy-to-use API for building complex filter conditions
- **Exception Handling**: Clear error messages for invalid operations and type mismatches
- **Zero Dependencies**: Core library has no external dependencies (tests and benchmarks use GoogleTest and Google Benchmark)

## Table of Contents

- [Requirements](#requirements)
- [Building](#building)
- [Usage](#usage)
- [API Reference](#api-reference)
- [Examples](#examples)
- [Testing](#testing)
- [Benchmarking](#benchmarking)
- [License](#license)

## Requirements

- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake 3.10** or higher
- **GoogleTest** (optional, for testing)
- **Google Benchmark** (optional, for benchmarks)

## Building

### Basic Build

```bash
mkdir build
cd build
cmake ..
make
```

### Build Options

You can customize the build with the following CMake options:

```bash
cmake -DBUILD_EXAMPLES=ON \    # Build example executables (default: ON)
      -DBUILD_BENCHMARKS=ON \  # Build benchmarks (default: ON, requires Google Benchmark)
      -DBUILD_TESTING=ON \     # Build tests (default: ON, requires GoogleTest)
      ..
```

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install libgtest-dev libbenchmark-dev
```

**macOS (with Homebrew):**
```bash
brew install googletest google-benchmark
```

## Usage

### Basic Example

```cpp
#include "evaluator.h"
#include <iostream>

int main() {
    // Create a filter condition: (A > 10) AND (B < 20)
    FilterCondition condition = {
        {
            { UnaryExpression{ComparisonOperations::GREATER_THAN, "A", int64_t(10)}, 
              LogicalOperations::NONE },
            { UnaryExpression{ComparisonOperations::LESS_THAN, "B", int64_t(20)}, 
              LogicalOperations::AND }
        }
    };

    // Initialize evaluator with the condition
    Evaluator evaluator;
    evaluator.initialize(condition);

    // Create key-value pairs to evaluate
    std::vector<Key> keys = {
        Key("A", int64_t(15)),
        Key("B", int64_t(15))
    };

    // Evaluate the condition
    bool result = evaluator.evaluate(keys);
    std::cout << "Evaluation result: " << (result ? "true" : "false") << std::endl;

    return 0;
}
```

### Advanced Example with Binary Expressions

```cpp
// Create a condition: (A + B < 100) AND (C == "test")
FilterCondition condition = {
    {
        { BinaryExpression{"A", ArithmeticOperations::ADD, "B", 
                          ComparisonOperations::LESS_THAN, int64_t(100)}, 
          LogicalOperations::NONE },
        { UnaryExpression{ComparisonOperations::EQUAL, "C", std::string("test")}, 
          LogicalOperations::AND }
    }
};

std::vector<Key> keys = {
    Key("A", int64_t(30)),
    Key("B", int64_t(40)),
    Key("C", std::string("test"))
};

auto eval_fn = LanguageParser::parse(condition);
bool result = eval_fn(keys);  // Returns true
```

## API Reference

### Core Classes

#### `Key`
Represents a key-value pair with a name and typed value.

```cpp
class Key {
public:
    Key(const std::string& name, const ValueType& value);
    const std::string& getName() const;
    const ValueType& getValue() const;
    void setValue(const ValueType& value);
};
```

#### `ValueType`
A variant type that can hold int64_t, double, string, or bool values.

```cpp
using ValueType = std::variant<int64_t, double, std::string, bool>;
```

#### `LanguageParser`
Static parser class that converts filter conditions into evaluable functions.

```cpp
class LanguageParser {
public:
    static std::function<bool(const std::vector<Key>&)> parse(const FilterCondition& condition);
};
```

#### `Evaluator`
Convenience wrapper around LanguageParser for easier usage.

```cpp
class Evaluator {
public:
    void initialize(const FilterCondition& condition);
    bool evaluate(const std::vector<Key>& keys);
};
```

### Expression Types

#### `UnaryExpression`
Simple comparison: `key op value`

```cpp
struct UnaryExpression {
    ComparisonOperations op;
    std::string key;
    ValueType value;
};
```

#### `BinaryExpression`
Arithmetic followed by comparison: `(key1 arith_op key2) comp_op value`

```cpp
struct BinaryExpression {
    std::string left_key;
    ArithmeticOperations arith_op;  // +, -, *, /
    std::string right_key;
    ComparisonOperations comp_op;   // ==, !=, >, <, >=, <=
    ValueType value;
};
```

### Operations

#### `ArithmeticOperations`
- `ADD` - Addition (+)
- `SUBTRACT` - Subtraction (-)
- `MULTIPLY` - Multiplication (*)
- `DIVIDE` - Division (/)

#### `ComparisonOperations`
- `EQUAL` - Equality (==)
- `NOT_EQUAL` - Inequality (!=)
- `GREATER_THAN` - Greater than (>)
- `LESS_THAN` - Less than (<)
- `GREATER_EQUAL` - Greater than or equal (>=)
- `LESS_EQUAL` - Less than or equal (<=)

#### `LogicalOperations`
- `AND` - Logical AND (&&)
- `OR` - Logical OR (||)
- `NONE` - No logical operation (for first expression)

## Examples

The `example/` directory contains working examples:

```bash
# Build and run examples
cd build
./basic
```

## Testing

The project uses GoogleTest for unit testing. Tests are located in the `test/` directory.

### Running Tests

```bash
# Install GoogleTest first
sudo apt-get install libgtest-dev  # Ubuntu/Debian

# Build with testing enabled
mkdir build && cd build
cmake -DBUILD_TESTING=ON ..
make

# Run tests
ctest
# or run individual test executables
./test_chatgpt
```

## Benchmarking

Performance benchmarks are available using Google Benchmark.

### Running Benchmarks

```bash
# Install Google Benchmark first
sudo apt-get install libbenchmark-dev  # Ubuntu/Debian

# Build with benchmarks enabled
mkdir build && cd build
cmake -DBUILD_BENCHMARKS=ON ..
make

# Run benchmarks
./chatgpt
```

## Project Structure

```
ExpressionEvaluator/
├── CMakeLists.txt          # Main build configuration
├── LICENSE                 # GPL-3.0 license
├── README.md              # This file
├── include/               # Public headers
│   ├── enums.h           # Operation enumerations
│   ├── evaluator.h       # High-level evaluator API
│   ├── filter_structs.h  # Filter condition structures
│   ├── key.h             # Key-value pair definition
│   └── parser.h          # Core parser interface
├── src/                   # Implementation files
│   └── parser.cpp        # Parser implementation
├── example/              # Usage examples
│   └── basic.cpp         # Basic usage example
├── test/                 # Unit tests
│   └── test_chatgpt.cpp  # Comprehensive test suite
└── benchmark/            # Performance benchmarks
    ├── CMakeLists.txt    # Benchmark build config
    └── chatgpt.cpp       # Benchmark suite
```

## Exception Handling

The library throws `ParseException` for various error conditions:

- **Type Mismatch**: Comparing values of different types
- **Division by Zero**: Arithmetic division by zero
- **Key Not Found**: Referenced key doesn't exist in the input
- **Unsupported Operations**: Invalid operation for a type (e.g., > on booleans)

Example:
```cpp
try {
    auto eval = LanguageParser::parse(condition);
    bool result = eval(keys);
} catch (const ParseException& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

## Performance Considerations

- The parser uses `std::variant` for type-safe value storage with minimal overhead
- Filter conditions are compiled once and can be reused for multiple evaluations
- No dynamic memory allocation during evaluation (after initial parsing)
- Zero-copy semantics where possible

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

### Development Guidelines

1. Follow the existing code style (C++17, consistent formatting)
2. Add tests for new features
3. Update documentation as needed
4. Ensure all tests pass before submitting

## Author

Project maintained by [akr-25](https://github.com/akr-25)

## Acknowledgments

- Built with modern C++17 features
- Uses GoogleTest for testing framework
- Uses Google Benchmark for performance testing
- 9 out of 9.11 words written in this repo is by AI.
