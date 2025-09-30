#include <gtest/gtest.h>

#include <string>
#include <variant>
#include <vector>

#include "enums.h"
#include "filter_structs.h"
#include "key.h"
#include "parser.h"

namespace {

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

  std::vector<Key> MakeKeys(std::initializer_list<Key> init) {
    return std::vector<Key>(init);
  }

  // ---------- Tests ----------

  TEST(LanguageParser_Basics, UnaryIntEqualTrueFalse) {
    auto keys = MakeKeys({Key("a", static_cast<int64_t>(5))});
    FilterCondition cond{
        {SE(UE(ComparisonOperations::EQUAL, "a", static_cast<int64_t>(5)))}};
    auto eval = LanguageParser::parse(cond);
    EXPECT_TRUE(eval(keys));
    keys[0].setValue(static_cast<int64_t>(6));
    EXPECT_FALSE(eval(keys));
  }

  TEST(LanguageParser_Basics, DoubleIntMixedArithmetic) {
    auto keys = MakeKeys({Key("x", static_cast<double>(2.5)),
                          Key("y", static_cast<int64_t>(3))});
    // (x + y) >= 5.0  -> 2.5 + 3 = 5.5 >= 5.0 => true
    FilterCondition cond{{SE(BE("x", ArithmeticOperations::ADD, "y",
                                ComparisonOperations::GREATER_EQUAL,
                                static_cast<double>(5.0)))}};
    auto eval = LanguageParser::parse(cond);
    EXPECT_TRUE(eval(keys));
  }

  TEST(LanguageParser_Basics, StringLexicographic) {
    auto keys = MakeKeys({Key("name", std::string("mango"))});
    // name < "z" -> true (lexicographic)
    FilterCondition cond{
        {SE(UE(ComparisonOperations::LESS_THAN, "name", std::string("z")))}};
    auto eval = LanguageParser::parse(cond);
    EXPECT_TRUE(eval(keys));
    keys[0].setValue(std::string("zzz"));
    EXPECT_FALSE(eval(keys));
  }

  TEST(LanguageParser_Logic, AndOrChaining) {
    auto keys = MakeKeys(
        {Key("a", static_cast<int64_t>(1)), Key("b", static_cast<int64_t>(2))});
    FilterCondition cond{
        {SE(UE(ComparisonOperations::EQUAL, "a", static_cast<int64_t>(1)),
            LogicalOperations::AND),
         SE(UE(ComparisonOperations::EQUAL, "b", static_cast<int64_t>(2)),
            LogicalOperations::OR),
         SE(UE(ComparisonOperations::NOT_EQUAL, "a", static_cast<int64_t>(0)))}};
    auto eval = LanguageParser::parse(cond);
    // (a==1) AND (b==2) OR (a!=0) -> true AND true OR true -> true
    EXPECT_TRUE(eval(keys));
  }

  TEST(LanguageParser_Errors, DivisionByZeroInt) {
    auto keys = MakeKeys({Key("x", static_cast<int64_t>(10)),
                          Key("y", static_cast<int64_t>(0))});
    FilterCondition cond{
        {SE(BE("x", ArithmeticOperations::DIVIDE, "y",
               ComparisonOperations::EQUAL, static_cast<int64_t>(0)))}};
    EXPECT_THROW(
        {
          auto eval = LanguageParser::parse(cond);
          eval(keys);
        },
        ParseException);
  }

  TEST(LanguageParser_Errors, TypeMismatchComparison) {
    auto keys = MakeKeys(
        {Key("a", static_cast<int64_t>(1)), Key("b", std::string("1"))});
    // a == "1" (int vs string) -> throws
    FilterCondition cond{
        {SE(UE(ComparisonOperations::EQUAL, "a", std::string("1")))}};
    auto eval = LanguageParser::parse(cond);
    EXPECT_THROW(eval(keys), ParseException);
  }

  TEST(LanguageParser_Errors, KeyNotFound) {
    auto keys = MakeKeys({Key("present", static_cast<int64_t>(1))});
    FilterCondition cond{{SE(
        UE(ComparisonOperations::EQUAL, "missing", static_cast<int64_t>(1)))}};
    auto eval = LanguageParser::parse(cond);
    EXPECT_THROW(eval(keys), ParseException);
  }

  TEST(LanguageParser_Boolean, BoolEqualityAllowedOnly) {
    auto keys = MakeKeys({Key("flag", true)});
    // EQUAL is allowed
    FilterCondition condEq{{SE(UE(ComparisonOperations::EQUAL, "flag", true))}};
    auto evalEq = LanguageParser::parse(condEq);
    EXPECT_TRUE(evalEq(keys));

    // GREATER_THAN on bool should throw per implementation
    FilterCondition condBad{
        {SE(UE(ComparisonOperations::GREATER_THAN, "flag", false))}};
    auto evalBad = LanguageParser::parse(condBad);
    EXPECT_THROW(evalBad(keys), ParseException);
  }

  TEST(LanguageParser_Integration, BinaryThenCompareWithConst) {
    auto keys = MakeKeys({Key("k0", static_cast<int64_t>(7)),
                          Key("k1", static_cast<int64_t>(6))});
    // (k0 * k1) == 42
    FilterCondition cond{
        {SE(BE("k0", ArithmeticOperations::MULTIPLY, "k1",
               ComparisonOperations::EQUAL, static_cast<int64_t>(42)))}};
    auto eval = LanguageParser::parse(cond);
    EXPECT_TRUE(eval(keys));
    keys[0].setValue(static_cast<int64_t>(8));
    EXPECT_FALSE(eval(keys));
  }

} // namespace
