#include "hxl-lang/services/processor.h"
#include "hxl-lang/services/deserializer.h"
#include "hxl-lang/services/parser.h"
#include "hxl-lang/services/schema-validator.h"
#include "hxl-lang/services/semantic-analyzer.h"
#include "hxl-lang/services/tokenizer.h"
#include "hxl-lang/services/transformer.h"

HXL::ProcessResult HXL::Processor::process(const std::string &source, const HXL::Schema &schema, const HXL::DeserializationProtocol &protocol) {
    PerformanceResults performanceResults;

    // Tokenization
    TokenizerResult tokenizerResult;
    performanceResults.tokenization = measure([&]() {
        tokenizerResult = Tokenizer::tokenize(source);
    });
    if (tokenizerResult.isErr()) {
        return {.errors = {tokenizerResult.error()}};
    }

    // Parsing
    Result<Document> syntaxTree;
    performanceResults.parsing = measure([&]() {
        syntaxTree = Parser::parse(tokenizerResult.get());
    });
    if (syntaxTree.isErr()) {
        return {.errors = {syntaxTree.error()}};
    }

    // Store document as a shared pointer
    std::shared_ptr<Document> document = std::make_shared<Document>(syntaxTree.get());

    // Semantic analysis
    ErrorList semanticErrors;
    performanceResults.semanticAnalysis = measure([&]() {
        semanticErrors = SemanticAnalyzer::analyze(document);
    });
    if (!semanticErrors.empty()) {
        return {.errors = semanticErrors};
    }

    // Transform (inheritance resolution, etc.)
    performanceResults.transformer = measure([&]() {
        Transformer::transform(document);
    });

    // Schema validation
    ErrorList schemaErrors;
    performanceResults.schemaValidation = measure([&]() {
        schemaErrors = SchemaValidator::validate(schema, document);
    });
    if (!schemaErrors.empty()) {
        return {.errors = schemaErrors};
    }

    // Deserialization
    ErrorList deserializationErrors;
    performanceResults.deserialization = measure([&]() {
      deserializationErrors = Deserializer::deserialize(protocol, document);
    });
    if (!deserializationErrors.empty()) {
        return {.errors = deserializationErrors};
    }

    return {
            .performanceResults = performanceResults,
    };
}
