#include <bbunit/bbunit.hpp>
#include <bbunit/utilities/printer.hpp>

#include <hxl-lang/hxl-lang.h>

#include "cases/base-case.cpp"
#include "cases/deserializer-test.cpp"
#include "cases/parser-test.cpp"
#include "cases/schema-validator-test.cpp"
#include "cases/semantic-analyzer-test.cpp"
#include "cases/transformer-test.cpp"
#include "cases/tokenizer-test.cpp"

int main() {
    BBUnit::TestResults results = BBUnit::TestRunner::run({
            std::make_shared<TokenizerTest>(TokenizerTest()),
            std::make_shared<ParserTest>(ParserTest()),
            std::make_shared<SemanticAnalyzerTest>(SemanticAnalyzerTest()),
            std::make_shared<DeserializerTest>(DeserializerTest()),
            std::make_shared<SchemaValidatorTest>(SchemaValidatorTest()),
            std::make_shared<TransformerTest>(TransformerTest()),
    });

    BBUnit::Utilities::Printer::print(results, {});
}
