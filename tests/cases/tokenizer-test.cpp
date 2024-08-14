#include <variant>

using namespace HXL;

using HXL::TokenType;

class TokenizerTest : public BaseCase {
public:
    /**
     * List of tests.
     */
    void test() override {
        nodeType();
        nodeProperty();
        unexpectedToken();
        reference();
        testTypes();
        inheritance();
        array();
        gen005_WhitespaceToTabs();
    }

    /**
     * Struct to aid in data type tests.
     */
    struct DataTypeTest {
        std::string value;
        HXL::TokenType tokenType;
    };

    /**
     * A helper function which makes testing the TokenizerResult safer,
     * by initially verifying the contents of std::variant.
     *
     * @param result
     * @param assertions
     */
    void assertTokenResult(const TokenizerResult &result,
                           const std::vector<Token> &expected) {
        assertFalse(result.isErr());
        std::vector<Token> actual = result.get();
        assertEquals<size_t>(expected.size(), actual.size());
        for (int i = 0; i <= expected.size() - 1; ++i) {
            assertEquals<HXL::TokenType>(expected[i].tokenType, actual[i].tokenType);
            assertEquals(expected[i].value.has_value(), actual[i].value.has_value());
            if (expected[i].value.has_value() && actual[i].value.has_value()) {
                assertEquals(expected[i].value.value(), actual[i].value.value());
            }
        }
    }

    /**
     * Iterate over the provided data type tests, and check for
     * each that the correct token has been detected.
     *
     * @param tests
     */
    void dataTypeTest(std::vector<DataTypeTest> tests) {
        std::for_each(tests.begin(), tests.end(), [&](const DataTypeTest &item) {
            assertTokenResult(
                    Tokenizer::tokenize("\tkey: " + item.value + "\n"),
                    {
                            {TokenType::T_TAB},
                            {TokenType::T_IDENTIFIER, "key"},
                            {TokenType::T_DELIMITER, ":"},
                            {TokenType::T_WHITESPACE},
                            {item.tokenType,
                             item.tokenType == HXL::TokenType::T_STRING_LITERAL
                                     ? item.value.substr(1, item.value.length() - 2)
                                     : item.value},
                            {TokenType::T_NEWLINE},
                    });
        });
    }

    /**
     * Node type
     */
    void nodeType() {
        it("Checks the node type", [&]() {
            assertTokenResult(
                    Tokenizer::tokenize("<NodeType> NodeName\n"),
                    {
                            {TokenType::T_DELIMITER, "<"},
                            {TokenType::T_IDENTIFIER, "NodeType"},
                            {TokenType::T_DELIMITER, ">"},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_IDENTIFIER, "NodeName"},
                            {TokenType::T_NEWLINE},
                    });
        });
    }

    /**
     * General test of parsing node properties
     */
    void nodeProperty() {
        it("Checks parsing of node properties.", [&]() {
            assertTokenResult(
                    Tokenizer::tokenize("\tkey: 5\n"),
                    {
                            {TokenType::T_TAB},
                            {TokenType::T_IDENTIFIER, "key"},
                            {TokenType::T_DELIMITER, ":"},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_INT, "5"},
                            {TokenType::T_NEWLINE},
                    });
        });
    }

    /**
     * Check that basic data types are mapped to the correct
     * token types.
     */
    void testTypes() {
        it("Tokenizes types such as int and string", [&]() {
            dataTypeTest({
                    DataTypeTest{.value = "5", .tokenType = HXL::TokenType::T_INT},
                    DataTypeTest{.value = "-5", .tokenType = HXL::TokenType::T_INT},
                    DataTypeTest{.value = "5.0", .tokenType = HXL::TokenType::T_FLOAT},
                    DataTypeTest{.value = "-5.0", .tokenType = HXL::TokenType::T_FLOAT},
                    DataTypeTest{.value = "true", .tokenType = HXL::TokenType::T_BOOL},
                    DataTypeTest{.value = "false", .tokenType = HXL::TokenType::T_BOOL},
                    DataTypeTest{.value = "HelloWorld", .tokenType = HXL::TokenType::T_IDENTIFIER},
                    DataTypeTest{.value = "\"\"", .tokenType = HXL::TokenType::T_STRING_LITERAL},
                    DataTypeTest{.value = "\"Hello, World!\"", .tokenType = HXL::TokenType::T_STRING_LITERAL},
            });
        });
    }

    /**
     * Verify that an "Unexpected token" error can occur.
     */
    void unexpectedToken() {
        it("Must cause a syntax error", [&]() {
            assertError(ErrorCode::HXL_SYNTAX_ERROR,
                        "[Line 1] Unexpected token: ?",
                        std::get<Error>(Tokenizer::tokenize("<Node?Type> A\n")));
        });
    }

    /**
     * Check a reference can be tokenized.
     */
    void reference() {
        it("Tokenizes references", [&]() {
            assertTokenResult(
                    Tokenizer::tokenize("\tkey&: RefName\n"),
                    {
                            {TokenType::T_TAB},
                            {TokenType::T_IDENTIFIER, "key"},
                            {TokenType::T_DELIMITER, "&"},
                            {TokenType::T_DELIMITER, ":"},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_IDENTIFIER, "RefName"},
                            {TokenType::T_NEWLINE},
                    });
        });
    }

    /**
     * Check that inheritance can be tokenized.
     */
    void inheritance() {
        it("Tokenizes inheritance delimiter", [&]() {
            assertTokenResult(
                    Tokenizer::tokenize("<NodeType> One <= Two\n"),
                    {
                            {TokenType::T_DELIMITER, "<"},
                            {TokenType::T_IDENTIFIER, "NodeType"},
                            {TokenType::T_DELIMITER, ">"},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_IDENTIFIER, "One"},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_DELIMITER, "<="},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_IDENTIFIER, "Two"},
                            {TokenType::T_NEWLINE},
                    });
        });
    }

    /**
     * Check than an array (in this case of 3 integers)
     * can be tokenized.
     */
    void array() {
        it("Tokenizes an array of three integers", [&]() {
            assertTokenResult(
                    Tokenizer::tokenize("\tkey[]: { 1, 2, 3 }\n"),
                    {
                            {TokenType::T_TAB},
                            {TokenType::T_IDENTIFIER, "key"},
                            {TokenType::T_DELIMITER, "[]"},
                            {TokenType::T_DELIMITER, ":"},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_PUNCTUATOR, "{"},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_INT, "1"},
                            {TokenType::T_DELIMITER, ","},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_INT, "2"},
                            {TokenType::T_DELIMITER, ","},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_INT, "3"},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_PUNCTUATOR, "}"},
                            {TokenType::T_NEWLINE},
                    });
        });
    }

    /**
     * GEN.005
     *
     * Check that four whitespace characters at the beginning of
     * a line are re-interpreted as a tab.
     */
    void gen005_WhitespaceToTabs() {
        it("Converts four white space characters to tabs", [&]() {
            assertTokenResult(
                    Tokenizer::tokenize("<NodeType> A\n    \n"),
                    {
                            {TokenType::T_DELIMITER, "<"},
                            {TokenType::T_IDENTIFIER, "NodeType"},
                            {TokenType::T_DELIMITER, ">"},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_IDENTIFIER, "A"},
                            {TokenType::T_NEWLINE},
                            {TokenType::T_TAB},
                            {TokenType::T_NEWLINE},
                    });
        });
    }
};
