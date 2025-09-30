#include "evaluator.h"
#include "filter_structs.h"
#include "key.h"
#include <benchmark/benchmark.h>
#include <random>
#include <vector>

// Helper function to generate random keys
std::vector<Key> generateRandomKeys(size_t count) {
  std::vector<Key> keys;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int64_t> int_dist(1, 1000);
  std::uniform_real_distribution<double> double_dist(1.0, 1000.0);

  for (size_t i = 0; i < count; ++i) {
    keys.emplace_back("key_" + std::to_string(i), int_dist(gen));
  }
  return keys;
}

// Benchmark: Simple unary expression evaluation
static void BM_SimpleUnaryExpression(benchmark::State &state) {
  // Setup: key1 == 500
  FilterCondition condition;
  UnaryExpression expr;
  expr.key = "key1";
  expr.op = ComparisonOperations::EQUAL;
  expr.value = int64_t(500);

  SubExpression subExpr;
  subExpr.expr = expr;
  subExpr.prev_logical_op = LogicalOperations::NONE;
  condition.sub_expressions.push_back(subExpr);

  Evaluator evaluator;
  evaluator.initialize(condition);

  std::vector<Key> keys = {Key("key1", int64_t(500)),
                           Key("key2", int64_t(100))};

  for (auto _ : state) {
    bool result = evaluator.evaluate(keys);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_SimpleUnaryExpression);

// Benchmark: Binary expression with arithmetic
static void BM_BinaryExpressionArithmetic(benchmark::State &state) {
  // Setup: key1 + key2 > 600
  FilterCondition condition;
  BinaryExpression expr;
  expr.left_key = "key1";
  expr.arith_op = ArithmeticOperations::ADD;
  expr.right_key = "key2";
  expr.comp_op = ComparisonOperations::GREATER_THAN;
  expr.value = int64_t(600);

  SubExpression subExpr;
  subExpr.expr = expr;
  subExpr.prev_logical_op = LogicalOperations::NONE;
  condition.sub_expressions.push_back(subExpr);

  Evaluator evaluator;
  evaluator.initialize(condition);

  std::vector<Key> keys = {Key("key1", int64_t(400)),
                           Key("key2", int64_t(300))};

  for (auto _ : state) {
    bool result = evaluator.evaluate(keys);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_BinaryExpressionArithmetic);

// Benchmark: Complex condition with AND operations
static void BM_ComplexConditionAND(benchmark::State &state) {
  // Setup: (key1 > 100) AND (key2 < 500) AND (key3 == 250)
  FilterCondition condition;

  // First expression
  UnaryExpression expr1;
  expr1.key = "key1";
  expr1.op = ComparisonOperations::GREATER_THAN;
  expr1.value = int64_t(100);
  SubExpression sub1;
  sub1.expr = expr1;
  sub1.prev_logical_op = LogicalOperations::NONE;
  condition.sub_expressions.push_back(sub1);

  // Second expression
  UnaryExpression expr2;
  expr2.key = "key2";
  expr2.op = ComparisonOperations::LESS_THAN;
  expr2.value = int64_t(500);
  SubExpression sub2;
  sub2.expr = expr2;
  sub2.prev_logical_op = LogicalOperations::AND;
  condition.sub_expressions.push_back(sub2);

  // Third expression
  UnaryExpression expr3;
  expr3.key = "key3";
  expr3.op = ComparisonOperations::EQUAL;
  expr3.value = int64_t(250);
  SubExpression sub3;
  sub3.expr = expr3;
  sub3.prev_logical_op = LogicalOperations::AND;
  condition.sub_expressions.push_back(sub3);

  Evaluator evaluator;
  evaluator.initialize(condition);

  std::vector<Key> keys = {Key("key1", int64_t(200)), Key("key2", int64_t(300)),
                           Key("key3", int64_t(250))};

  for (auto _ : state) {
    bool result = evaluator.evaluate(keys);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_ComplexConditionAND);

// Benchmark: Complex condition with OR operations
static void BM_ComplexConditionOR(benchmark::State &state) {
  // Setup: (key1 > 1000) OR (key2 < 50) OR (key3 == 250)
  FilterCondition condition;

  UnaryExpression expr1;
  expr1.key = "key1";
  expr1.op = ComparisonOperations::GREATER_THAN;
  expr1.value = int64_t(1000);
  SubExpression sub1;
  sub1.expr = expr1;
  sub1.prev_logical_op = LogicalOperations::NONE;
  condition.sub_expressions.push_back(sub1);

  UnaryExpression expr2;
  expr2.key = "key2";
  expr2.op = ComparisonOperations::LESS_THAN;
  expr2.value = int64_t(50);
  SubExpression sub2;
  sub2.expr = expr2;
  sub2.prev_logical_op = LogicalOperations::OR;
  condition.sub_expressions.push_back(sub2);

  UnaryExpression expr3;
  expr3.key = "key3";
  expr3.op = ComparisonOperations::EQUAL;
  expr3.value = int64_t(250);
  SubExpression sub3;
  sub3.expr = expr3;
  sub3.prev_logical_op = LogicalOperations::OR;
  condition.sub_expressions.push_back(sub3);

  Evaluator evaluator;
  evaluator.initialize(condition);

  std::vector<Key> keys = {Key("key1", int64_t(200)), Key("key2", int64_t(300)),
                           Key("key3", int64_t(250))};

  for (auto _ : state) {
    bool result = evaluator.evaluate(keys);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_ComplexConditionOR);

// Benchmark: Mixed logical operations
static void BM_MixedLogicalOperations(benchmark::State &state) {
  // Setup: (key1 > 100) AND (key2 < 500) OR (key3 == 250)
  FilterCondition condition;

  UnaryExpression expr1;
  expr1.key = "key1";
  expr1.op = ComparisonOperations::GREATER_THAN;
  expr1.value = int64_t(100);
  SubExpression sub1;
  sub1.expr = expr1;
  sub1.prev_logical_op = LogicalOperations::NONE;
  condition.sub_expressions.push_back(sub1);

  UnaryExpression expr2;
  expr2.key = "key2";
  expr2.op = ComparisonOperations::LESS_THAN;
  expr2.value = int64_t(500);
  SubExpression sub2;
  sub2.expr = expr2;
  sub2.prev_logical_op = LogicalOperations::AND;
  condition.sub_expressions.push_back(sub2);

  UnaryExpression expr3;
  expr3.key = "key3";
  expr3.op = ComparisonOperations::EQUAL;
  expr3.value = int64_t(250);
  SubExpression sub3;
  sub3.expr = expr3;
  sub3.prev_logical_op = LogicalOperations::OR;
  condition.sub_expressions.push_back(sub3);

  Evaluator evaluator;
  evaluator.initialize(condition);

  std::vector<Key> keys = {Key("key1", int64_t(200)), Key("key2", int64_t(300)),
                           Key("key3", int64_t(250))};

  for (auto _ : state) {
    bool result = evaluator.evaluate(keys);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_MixedLogicalOperations);

// Benchmark: String comparison
static void BM_StringComparison(benchmark::State &state) {
  FilterCondition condition;

  UnaryExpression expr;
  expr.key = "name";
  expr.op = ComparisonOperations::EQUAL;
  expr.value = std::string("test_value");

  SubExpression subExpr;
  subExpr.expr = expr;
  subExpr.prev_logical_op = LogicalOperations::NONE;
  condition.sub_expressions.push_back(subExpr);

  Evaluator evaluator;
  evaluator.initialize(condition);

  std::vector<Key> keys = {Key("name", std::string("test_value")),
                           Key("id", int64_t(123))};

  for (auto _ : state) {
    bool result = evaluator.evaluate(keys);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_StringComparison);

// Benchmark: Double arithmetic operations
static void BM_DoubleArithmetic(benchmark::State &state) {
  FilterCondition condition;

  BinaryExpression expr;
  expr.left_key = "value1";
  expr.arith_op = ArithmeticOperations::MULTIPLY;
  expr.right_key = "value2";
  expr.comp_op = ComparisonOperations::GREATER_EQUAL;
  expr.value = 500.0;

  SubExpression subExpr;
  subExpr.expr = expr;
  subExpr.prev_logical_op = LogicalOperations::NONE;
  condition.sub_expressions.push_back(subExpr);

  Evaluator evaluator;
  evaluator.initialize(condition);

  std::vector<Key> keys = {Key("value1", 25.5), Key("value2", 20.0)};

  for (auto _ : state) {
    bool result = evaluator.evaluate(keys);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_DoubleArithmetic);

// Benchmark: Varying number of keys (parameter sweep)
static void BM_VaryingKeyCount(benchmark::State &state) {
  size_t key_count = state.range(0);

  FilterCondition condition;
  UnaryExpression expr;
  expr.key = "key_5";
  expr.op = ComparisonOperations::GREATER_THAN;
  expr.value = int64_t(500);

  SubExpression subExpr;
  subExpr.expr = expr;
  subExpr.prev_logical_op = LogicalOperations::NONE;
  condition.sub_expressions.push_back(subExpr);

  Evaluator evaluator;
  evaluator.initialize(condition);

  std::vector<Key> keys = generateRandomKeys(key_count);

  for (auto _ : state) {
    bool result = evaluator.evaluate(keys);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_VaryingKeyCount)->Range(8, 8 << 10);

// Benchmark: Varying number of subexpressions
static void BM_VaryingExpressionCount(benchmark::State &state) {
  size_t expr_count = state.range(0);

  FilterCondition condition;

  for (size_t i = 0; i < expr_count; ++i) {
    UnaryExpression expr;
    expr.key = "key_" + std::to_string(i % 10);
    expr.op = ComparisonOperations::LESS_THAN;
    expr.value = int64_t(800);

    SubExpression subExpr;
    subExpr.expr = expr;
    subExpr.prev_logical_op =
        (i == 0) ? LogicalOperations::NONE : LogicalOperations::AND;
    condition.sub_expressions.push_back(subExpr);
  }

  Evaluator evaluator;
  evaluator.initialize(condition);

  std::vector<Key> keys = generateRandomKeys(10);

  for (auto _ : state) {
    bool result = evaluator.evaluate(keys);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_VaryingExpressionCount)->Range(1, 64);

// Benchmark: All arithmetic operations
static void BM_AllArithmeticOps(benchmark::State &state) {
  ArithmeticOperations ops[] = {
      ArithmeticOperations::ADD, ArithmeticOperations::SUBTRACT,
      ArithmeticOperations::MULTIPLY, ArithmeticOperations::DIVIDE};

  ArithmeticOperations current_op = ops[state.range(0)];

  FilterCondition condition;
  BinaryExpression expr;
  expr.left_key = "a";
  expr.arith_op = current_op;
  expr.right_key = "b";
  expr.comp_op = ComparisonOperations::GREATER_THAN;
  expr.value = int64_t(100);

  SubExpression subExpr;
  subExpr.expr = expr;
  subExpr.prev_logical_op = LogicalOperations::NONE;
  condition.sub_expressions.push_back(subExpr);

  Evaluator evaluator;
  evaluator.initialize(condition);

  std::vector<Key> keys = {Key("a", int64_t(500)), Key("b", int64_t(200))};

  for (auto _ : state) {
    bool result = evaluator.evaluate(keys);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_AllArithmeticOps)->DenseRange(0, 3);

BENCHMARK_MAIN();