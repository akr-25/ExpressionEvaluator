#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

// Your project headers
#include "enums.h"
#include "filter_structs.h"
#include "key.h"
#include "parser.h"

namespace {

  // Helpers to build expressions/conditions
  // ------------------------------------
  UnaryExpression UE(ComparisonOperations op, std::string key, ValueType val) {
    return UnaryExpression{op, std::move(key), std::move(val)};
  }

  BinaryExpression BE(std::string left_key, ArithmeticOperations aop,
                      std::string right_key, ComparisonOperations cop,
                      ValueType val) {
    return BinaryExpression{std::move(left_key), aop, std::move(right_key), cop,
                            std::move(val)};
  }

  SubExpression SE(UnaryExpression ue,
                   LogicalOperations next = LogicalOperations::NONE) {
    return SubExpression{
        std::variant<UnaryExpression, BinaryExpression>{std::move(ue)}, next};
  }

  SubExpression SE(BinaryExpression be,
                   LogicalOperations next = LogicalOperations::NONE) {
    return SubExpression{
        std::variant<UnaryExpression, BinaryExpression>{std::move(be)}, next};
  }

  std::vector<Key> MakeSequentialIntKeys(std::size_t n) {
    std::vector<Key> keys;
    keys.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      keys.emplace_back("k" + std::to_string(i), static_cast<int64_t>(i));
    }
    return keys;
  }

  std::vector<Key> MakeMixedKeys(std::size_t n) {
    std::vector<Key> keys;
    keys.reserve(n);
    // Cycle through int, double, string, bool
    for (std::size_t i = 0; i < n; ++i) {
      switch (i % 4) {
      case 0:
        keys.emplace_back("k" + std::to_string(i), static_cast<int64_t>(i));
        break;
      case 1:
        keys.emplace_back("k" + std::to_string(i),
                          static_cast<double>(i) + 0.5);
        break;
      case 2:
        keys.emplace_back("k" + std::to_string(i),
                          std::string("str") + std::to_string(i));
        break;
      case 3:
        keys.emplace_back("k" + std::to_string(i), (i % 2) == 0);
        break;
      }
    }
    return keys;
  }

  // Bench 1: simple unary int comparison
  // ---------------------------------------
  static void BM_UnaryIntEqual(benchmark::State & state) {
    const std::size_t key_count = static_cast<std::size_t>(state.range(0));
    auto keys = MakeSequentialIntKeys(std::max<std::size_t>(key_count, 1));

    FilterCondition cond{
        {SE(UE(ComparisonOperations::EQUAL, "k0", static_cast<int64_t>(42)))}};
    auto eval = LanguageParser::parse(cond);

    // Vary k0 so we don't constant-fold everything
    int64_t v = 0;
    for (auto _ : state) {
      v += 1;
      keys[0].setValue(v % 100); // range 0..99
      benchmark::DoNotOptimize(eval(keys));
    }
  }
  BENCHMARK(BM_UnaryIntEqual)->Arg(4)->Arg(16)->Arg(64);

  // Bench 2: binary arithmetic (k0 + k1) < const with AND chain length N
  // --------
  static void BM_BinaryArithAndChain(benchmark::State & state) {
    const int chain_len = static_cast<int>(state.range(0));
    auto keys = MakeSequentialIntKeys(std::max(2, chain_len + 1));

    FilterCondition cond;
    cond.sub_expressions.reserve(chain_len);
    for (int i = 0; i < chain_len; ++i) {
      // (k0 + k1) < 100, repeated and ANDed
      auto be = BE("k0", ArithmeticOperations::ADD, "k1",
                   ComparisonOperations::LESS_THAN, static_cast<int64_t>(100));
      cond.sub_expressions.push_back(
          SE(std::move(be),
             i == chain_len - 1 ? LogicalOperations::NONE : LogicalOperations::AND));
    }

    auto eval = LanguageParser::parse(cond);
    int64_t tick = 0;
    for (auto _ : state) {
      // Change values so branches are mixed
      tick += 3;
      keys[0].setValue((tick % 200) - 50); // -50..149
      keys[1].setValue((tick % 150));      // 0..149
      benchmark::DoNotOptimize(eval(keys));
    }
  }
  BENCHMARK(BM_BinaryArithAndChain)->Arg(1)->Arg(4)->Arg(8)->Arg(16)->Arg(32);

  // Bench 3: OR-heavy chain with mixed types
  // ------------------------------------
  static void BM_MixedTypesOrChain(benchmark::State & state) {
    const int chain_len = static_cast<int>(state.range(0));
    auto keys = MakeMixedKeys(std::max(8, chain_len + 4));

    FilterCondition cond;
    for (int i = 0; i < chain_len; ++i) {
      if (i % 3 == 0) {
        cond.sub_expressions.push_back(
            SE(UE(ComparisonOperations::GREATER_THAN, "k1", 0.25),
               i == chain_len - 1 ? LogicalOperations::NONE : LogicalOperations::OR));
      } else if (i % 3 == 1) {
        cond.sub_expressions.push_back(
            SE(UE(ComparisonOperations::LESS_THAN, "k0",
                  static_cast<int64_t>(100)),
               i == chain_len - 1 ? LogicalOperations::NONE : LogicalOperations::OR));
      } else {
        cond.sub_expressions.push_back(
            SE(UE(ComparisonOperations::NOT_EQUAL, "k2", std::string("str999")),
               i == chain_len - 1 ? LogicalOperations::NONE : LogicalOperations::OR));
      }
    }

    auto eval = LanguageParser::parse(cond);
    double dv = 0.0;
    int64_t iv = 0;
    int sidx = 0;
    for (auto _ : state) {
      dv += 0.1;
      iv += 1;
      sidx = (sidx + 1) % 1000;
      keys[1].setValue(dv);
      keys[0].setValue(iv);
      keys[2].setValue(std::string("str") + std::to_string(sidx));
      benchmark::DoNotOptimize(eval(keys));
    }
  }
  BENCHMARK(BM_MixedTypesOrChain)->Arg(1)->Arg(4)->Arg(8)->Arg(16)->Arg(32);

  // Bench 4: String lexicographic comparisons
  // -----------------------------------
  static void BM_StringCompareChain(benchmark::State & state) {
    const int chain_len = static_cast<int>(state.range(0));
    std::vector<Key> keys;
    keys.emplace_back("name", std::string("alpha"));

    FilterCondition cond;
    for (int i = 0; i < chain_len; ++i) {
      cond.sub_expressions.push_back(
          SE(UE(ComparisonOperations::LESS_THAN, "name", std::string("m")),
             i == chain_len - 1 ? LogicalOperations::NONE : LogicalOperations::AND));
    }

    auto eval = LanguageParser::parse(cond);
    const char *words[] = {"alpha", "delta", "kilo", "omega", "zulu"};
    int idx = 0;
    for (auto _ : state) {
      idx = (idx + 1) % 5;
      keys[0].setValue(std::string(words[idx]));
      benchmark::DoNotOptimize(eval(keys));
    }
  }
  BENCHMARK(BM_StringCompareChain)->Arg(1)->Arg(4)->Arg(8)->Arg(16)->Arg(32);

  // Bench 5: Large key set, sparse access
  // ---------------------------------------
  static void BM_LargeKeysetSparse(benchmark::State & state) {
    const std::size_t key_count = static_cast<std::size_t>(state.range(0));
    auto keys = MakeSequentialIntKeys(std::max<std::size_t>(key_count, 4));

    // Only touch a few keys in the condition
    FilterCondition cond{
        {SE(UE(ComparisonOperations::GREATER_EQUAL, "k10",
               static_cast<int64_t>(5))),
         SE(UE(ComparisonOperations::LESS_EQUAL, "k20",
               static_cast<int64_t>(500)),
            LogicalOperations::AND),
         SE(BE("k1", ArithmeticOperations::MULTIPLY, "k2",
               ComparisonOperations::EQUAL, static_cast<int64_t>(42)),
            LogicalOperations::AND)}};

    auto eval = LanguageParser::parse(cond);
    int64_t t = 0;
    for (auto _ : state) {
      t += 7;
      if (keys.size() > 21) {
        keys[10].setValue((t % 13));
        keys[20].setValue((t % 1000));
      }
      keys[1].setValue((t % 7));
      keys[2].setValue((t % 6));
      benchmark::DoNotOptimize(eval(keys));
    }
  }
  BENCHMARK(BM_LargeKeysetSparse)->Arg(32)->Arg(128)->Arg(512)->Arg(2048);

  // Bench 6: Build-time cost of parsing large conditions
  // -------------------------
  static void BM_ParseCost(benchmark::State & state) {
    const int chain_len = static_cast<int>(state.range(0));
    for (auto _ : state) {
      state.PauseTiming();
      FilterCondition cond;
      cond.sub_expressions.reserve(chain_len);
      for (int i = 0; i < chain_len; ++i) {
        auto prev =
            (i == 0) ? LogicalOperations::NONE
                     : (i % 2 ? LogicalOperations::AND : LogicalOperations::OR);
        cond.sub_expressions.push_back(
            SE(UE(ComparisonOperations::NOT_EQUAL, "k" + std::to_string(i % 8),
                  static_cast<int64_t>(i)),
               prev));
      }
      state.ResumeTiming();
      auto eval = LanguageParser::parse(cond);
      benchmark::DoNotOptimize(eval);
    }
  }
  BENCHMARK(BM_ParseCost)->Arg(8)->Arg(32)->Arg(128)->Arg(512)->Arg(2048);

} // namespace

BENCHMARK_MAIN();