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
        comments();

        gen005_WhitespaceToTabs();
        cmt002_WhitespaceAroundComment();
        str002_HashWithinStringLiteral();
        str003_ColonWithinStringLiteral();
        str004_LineBreakWithinStringLiteral();
        cmt003_EmptyComment();
        cmt004_WhitespaceCommentsStandAlone();
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
     * Comments
     *
     * Check that comments are not interpreted as tokens
     */
    void comments() {
        it("Should ignore comments at the end of a line.", [&]() {
            assertTokenResult(
                    Tokenizer::tokenize("100 # This is a comment\n"),
                    {
                            {TokenType::T_INT, "100"},
                            {TokenType::T_NEWLINE},
                    });
        });

        it("Should ignore comments on a stand-alone line.", [&]() {
            assertTokenResult(
                    Tokenizer::tokenize("# This is a comment\n"),
                    {
                            {TokenType::T_NEWLINE}
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

    /**
     * CMT.002
     *
     * There must be exactly one whitespace character before and after the #.
     */
    void cmt002_WhitespaceAroundComment() {
        it("Ensures that there's whitespace in front of the #", [&]() {
            auto result = Tokenizer::tokenize("100# Comment here\n");
            assertError(ErrorCode::HXL_ILLEGAL_WHITESPACE,
                        "[Line 1, Col 3] Illegal whitespace",
                        result.error());
        });

        it("Ensures that there's whitespace in after the #", [&]() {
            auto result = Tokenizer::tokenize("100 #Comment here\n");
            assertError(ErrorCode::HXL_ILLEGAL_WHITESPACE,
                        "[Line 1, Col 4] Illegal whitespace",
                        result.error());
        });
    }

    /**
     * CMT.004
     *
     * There must be exactly one whitespace character after #
     * Whitespace is not allowed in front of the #
     */
    void cmt004_WhitespaceCommentsStandAlone() {
        it("There must not be whitespace before # when single-line comment", [&]() {
            auto result = Tokenizer::tokenize(" # Comment here\n");
            assertError(ErrorCode::HXL_ILLEGAL_WHITESPACE,
                        "[Line 1, Col 0] Illegal whitespace",
                        result.error());
        });

        it("There must be whitespace after # when single-line comment", [&]() {
            auto result = Tokenizer::tokenize("#Comment here\n");
            assertError(ErrorCode::HXL_ILLEGAL_WHITESPACE,
                        "[Line 1, Col 0] Illegal whitespace",
                        result.error());
        });
    }

    /**
     * CMT.003
     *
     * Comments cannot be empty
     */
    void cmt003_EmptyComment() {
        it("Must cause an error, if a comment is empty (Stand-alone, no whitespace).", [&]() {
            auto result = Tokenizer::tokenize("#\n");
            assertError(ErrorCode::HXL_ILLEGAL_WHITESPACE,
                        "[Line 1, Col 0] Illegal whitespace",
                        result.error());
        });

        it("Must cause an error, if a comment is empty (Stand-alone).", [&]() {
            auto result = Tokenizer::tokenize("# \n");
            assertError(ErrorCode::HXL_ILLEGAL_COMMENT,
                        "[Line 1] Illegal comment",
                        result.error());
        });

        it("Must cause an error, if a comment is empty (End of line, no whitespace).", [&]() {
            auto result = Tokenizer::tokenize("100 #\n");
            assertError(ErrorCode::HXL_ILLEGAL_WHITESPACE,
                        "[Line 1, Col 4] Illegal whitespace",
                        result.error());
        });

        it("Must cause an error, if a comment is empty (End of line).", [&]() {
            auto result = Tokenizer::tokenize("100 # \n");
            assertError(ErrorCode::HXL_ILLEGAL_COMMENT,
                        "[Line 1] Illegal comment",
                        result.error());
        });
    }

    /**
     * STR.002
     */
    void str002_HashWithinStringLiteral() {
        it("Must be accepted that hashtags can be part of a string.", [&]() {
            assertTokenResult(
                    Tokenizer::tokenize("\tkey: \"Hello # World\"\n"),
                    {
                            {TokenType::T_TAB},
                            {TokenType::T_IDENTIFIER, "key"},
                            {TokenType::T_DELIMITER, ":"},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_STRING_LITERAL, "Hello # World"},
                            {TokenType::T_NEWLINE},
                    });
        });
    }

    /**
     * STR.003
     */
    void str003_ColonWithinStringLiteral() {
        it("Must be accepted that colon can be part of a string.", [&]() {
            assertTokenResult(
                    Tokenizer::tokenize("\tkey: \"Hello : World\"\n"),
                    {
                            {TokenType::T_TAB},
                            {TokenType::T_IDENTIFIER, "key"},
                            {TokenType::T_DELIMITER, ":"},
                            {TokenType::T_WHITESPACE},
                            {TokenType::T_STRING_LITERAL, "Hello : World"},
                            {TokenType::T_NEWLINE},
                    });
        });
    }

    /**
     * STR.004
     */
    void str004_LineBreakWithinStringLiteral() {
        it("Must be accepted that line-breaks can be part of a string.", [&]() {
            auto result = Tokenizer::tokenize("\tkey: \"Hello \n World\"\n");
            assertError(ErrorCode::HXL_ILLEGAL_WHITESPACE,
                        "[Line 1, Col 13] Illegal whitespace",
                        result.error());
        });
    }
};
