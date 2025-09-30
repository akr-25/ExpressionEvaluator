#Google Benchmark framework for `LanguageParser`

This mini framework sets up Google Benchmark and adds a handful of realistic microbenchmarks for your parser/evaluator. It assumes your headers (e.g., `parser.h`, `filter_structs.h`, `key.h`, `enums.h`) are available and that the implementation for `LanguageParser` is compiled into the benchmark target (either as separate sources or via a library you link).

Usage of AI:
High