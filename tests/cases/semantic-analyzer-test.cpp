using namespace HXL;

class SemanticAnalyzerTest : public BaseCase {
public:
    /**
     * List of tests.
     */
    void test() override {
        node200_UniqueNodeName();
        node201_UniquePropertyName();
    }

    /**
     * Check that node names are unique.
     */
    void node200_UniqueNodeName() {
        it("Checks that node names are unique", [&]() {
            auto tokens = Tokenizer::tokenize("<NodeType> A\n\n<NodeType> B\n\n<NodeType> A\n");
            Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));
            std::vector<Error> errors = SemanticAnalyzer::analyze(std::make_shared<Document>(syntaxTree.get()));

            assertCount(1, errors);
            assertEquals<ErrorCode>(ErrorCode::HXL_NON_UNIQUE_NODE, errors[0].errorCode);
            assertEquals<std::string>("Node name \"A\" is not unique.", errors[0].message);
        });
    }

    /**
     * Check that the properties under a node are unique.
     */
    void node201_UniquePropertyName() {
        it("Checks that property names are unique", [&]() {
            auto tokens = Tokenizer::tokenize("<NodeType> A\n\ta: 8\n\tb: 10\n\ta: 11\n");
            Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));
            std::vector<Error> errors = SemanticAnalyzer::analyze(std::make_shared<Document>(syntaxTree.get()));

            assertCount(1, errors);
            assertEquals<ErrorCode>(ErrorCode::HXL_NON_UNIQUE_PROPERTY, errors[0].errorCode);
            assertEquals<std::string>(R"(Property "a" under "A" is not unique.)", errors[0].message);
        });
    }
};
