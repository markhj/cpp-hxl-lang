using namespace HXL;

class ParserTest : public BaseCase {
public:
    /**
     * List of tests
     */
    void test() override {
        validNode();
        validProperties();
        unexpectedToken();
        reference();
        inheritance();
        array();

        gen001_Empty();
        gen002_InvalidEOF();
        node002_WhitespaceBetweenNodeTypeAndName();
        node003_TabBeforeProperty();
        node005_WhitespaceBetweenKeyAndColon();
        node006_WhitespaceAfterKeyAndColon();
        node015_WhitespaceAfterValue();
    }

    /**
     * Helper method to check a document.
     *
     * Has a couple of safety mechanisms for when the syntax tree
     * is different than expected.
     *
     * @param tokenizerResult
     * @param expectedNodes
     */
    void assertDocument(const Result<std::vector<Token>> &tokenizerResult,
                        const std::vector<Node> &expectedNodes) {
        assertFalse(tokenizerResult.isErr()).because("Tokenization must succeed.");

        // Display the error, as this is unexpected
        if (tokenizerResult.isErr()) {
            std::cerr << "ERR: " << tokenizerResult.error().message << std::endl;
        }

        std::variant<Error, Document> result = Parser::parse(std::get<std::vector<Token>>(tokenizerResult));

        assertFalse(std::holds_alternative<Error>(result)).because("Document must not contain errors.");

        // Display the parser error, as this would be unexpected
        if (std::holds_alternative<Error>(result)) {
            std::cerr << "ERR: " << std::get<Error>(result).message << std::endl;
        }

        Document document = std::get<Document>(result);

        assertEquals((int) expectedNodes.size(),
                     (int) document.nodes.size())
                .because("Number of nodes must be equal.");

        // If the number of elements differ, there's no point in continuing
        // to assert, and it might also cause access violations.
        if (expectedNodes.size() != document.nodes.size()) {
            return;
        }

        for (int i = 0; i < expectedNodes.size(); ++i) {
            assertEquals(expectedNodes[i].type, document.nodes[i].type).because("Types must match");
            assertEquals<std::string>(expectedNodes[i].name, document.nodes[i].name).because("Names must match");
            assertEquals(expectedNodes[i].inheritance.has_value(),
                         document.nodes[i].inheritance.has_value())
                    .because("Must have same inheritance");

            if (expectedNodes[i].inheritance.has_value() && document.nodes[i].inheritance.has_value()) {
                assertEquals(expectedNodes[i].inheritance.value().from,
                             document.nodes[i].inheritance.value().from)
                        .because("Must inherit from same node name");
            }

            assertEquals<int>((int) expectedNodes[i].properties.size(),
                              (int) document.nodes[i].properties.size())
                    .because("Number of node properties must be equal.");

            if (expectedNodes[i].properties.size() == document.nodes[i].properties.size()) {
                for (int j = 0; j < expectedNodes[i].properties.size(); ++j) {
                    assertEquals(expectedNodes[i].properties[j].name,
                                 document.nodes[i].properties[j].name)
                            .because("Property names must match");

                    // @todo For arrays, expand to compare all values
                    assertEquals<size_t>(expectedNodes[i].properties[j].values.size(),
                                         document.nodes[i].properties[j].values.size())
                            .because("Number of properties must be identical");

                    for (int h = 0; h < expectedNodes[i].properties[j].values.size(); ++h) {
                        assertEquals(expectedNodes[i].properties[j].values[h],
                                     document.nodes[i].properties[j].values[h])
                                .because(std::format("Property value {} must match", expectedNodes[i].properties[j].name));
                    }

                    assertEquals(expectedNodes[i].properties[j].dataType,
                                 document.nodes[i].properties[j].dataType)
                            .because(std::format("Data type for property {} must match", expectedNodes[i].properties[j].name));
                }
            }
        }
    }

    /**
     * Verify that an empty document causes an error.
     */
    void gen001_Empty() {
        it("Fails, because the source is empty", [&]() {
            auto tokens = Tokenizer::tokenize("");
            assertError(ErrorCode::HXL_EMPTY,
                        "Source is empty.",
                        std::get<Error>(Parser::parse(std::get<std::vector<Token>>(tokens))));
        });
    }

    /**
     * Verify that an empty line must be present at the end of the source.
     */
    void gen002_InvalidEOF() {
        it("Fails because there's no empty line at the end of file", [&]() {
            auto tokens = Tokenizer::tokenize("<NodeType> A");
            assertError(ErrorCode::HXL_INVALID_EOF,
                        "Source must end with an empty line.",
                        std::get<Error>(Parser::parse(std::get<std::vector<Token>>(tokens))));
        });
    }

    /**
     * Example of a valid node.
     */
    void validNode() {
        it("Parses a valid node", [&]() {
            assertDocument(
                    Tokenizer::tokenize("<Cube> A\n\n<Sphere> B\n"),
                    {
                            {"Cube", "A", {}},
                            {"Sphere", "B", {}},
                    });
        });
    }

    /**
     * Set of valid properties.
     *
     * A couple of basic data types are tested.
     */
    void validProperties() {
        it("Parses valid properties", [&]() {
            assertDocument(
                    Tokenizer::tokenize("<Sphere> MySphere\n\tradius: 8\n"),
                    {
                            {"Sphere", "MySphere", {NodeProperty{"radius", {"8"}, DataType::Int}}},
                    });

            assertDocument(
                    Tokenizer::tokenize("<NodeType> A\n\tstring: \"Hello, World!\"\n\tint: 12\n\tfloat: 8.05\n\tbool: true\n"),
                    {
                            {"NodeType",
                             "A",
                             {NodeProperty{"string", {"Hello, World!"}, DataType::String},
                              NodeProperty{"int", {"12"}, DataType::Int},
                              NodeProperty{"float", {"8.05"}, DataType::Float},
                              NodeProperty{"bool", {"true"}, DataType::Bool}}},
                    });
        });
    }

    /**
     * Check that an unexpected error is thrown, when one is encountered.
     */
    void unexpectedToken() {
        it("Finds unexpected token during parsing", [&]() {
            auto tokens = Tokenizer::tokenize("<NodeType> A\n<NodeType>: B\n");
            assertError(ErrorCode::HXL_UNEXPECTED_TOKEN,
                        "[Line 2, Col 10] Unexpected token: :",
                        std::get<Error>(Parser::parse(std::get<std::vector<Token>>(tokens))));
        });
    }

    /**
     * Check parsing of a reference.
     *
     * What is expected is simply to say "there's a reference to a node."
     * It's a later stage which validates if the node exists.
     */
    void reference() {
        it("Parses references", [&]() {
            assertDocument(
                    Tokenizer::tokenize("<Sphere> MySphere\n\tref&: RefName\n"),
                    {
                            {"Sphere", "MySphere", {NodeProperty{"ref", {"RefName"}, DataType::NodeRef}}},

                    });
        });
    }

    /**
     * Check that inheritance can be parsed.
     */
    void inheritance() {
        it("Parses inheritance", [&]() {
            assertDocument(
                    Tokenizer::tokenize("<Sphere> A <= B\n"),
                    {
                            {"Sphere", "A", {}, Inheritance{.from = "B"}},
                    });
        });
    }

    /**
     * Check parsing of array types.
     */
    void array() {
        it("Parses arrays with ints", [&]() {
            assertDocument(
                    Tokenizer::tokenize("<Sphere> A\n\tarr[]: { 1, 2, 3 }\n"),
                    {
                            {"Sphere",
                             "A",
                             {NodeProperty{"arr", {"1", "2", "3"}, DataType::Int}}},
                    });
        });

        it("Parses arrays with floats", [&]() {
            assertDocument(
                    Tokenizer::tokenize("<Sphere> A\n\tarr[]: { 1.5, 2.5, -3.5 }\n"),
                    {
                            {"Sphere",
                             "A",
                             {NodeProperty{"arr", {"1.5", "2.5", "-3.5"}, DataType::Float}}},
                    });
        });

        it("Parses arrays with strings", [&]() {
            assertDocument(
                    Tokenizer::tokenize("<Sphere> A\n\tarr[]: { \"a\", \"b\", \"c\" }\n"),
                    {
                            {"Sphere",
                             "A",
                             {NodeProperty{"arr", {"a", "b", "c"}, DataType::String}}},
                    });
        });
    }

    /**
     * NODE.002
     *
     * Whitespace between node type and name.
     */
    void node002_WhitespaceBetweenNodeTypeAndName() {
        it("There must be whitespace between node type and name", [&]() {
            auto tokens = Tokenizer::tokenize("<NodeType>A\n");
            assertError(ErrorCode::HXL_UNEXPECTED_TOKEN,
                        "[Line 1, Col 10] Unexpected token: A",
                        std::get<Error>(Parser::parse(std::get<std::vector<Token>>(tokens))));
        });
    }

    /**
     * NODE.003
     *
     * Tab before property.
     */
    void node003_TabBeforeProperty() {
        it("There must be a tab before the property.", [&]() {
            auto tokens = Tokenizer::tokenize("a: key\n");
            assertError(ErrorCode::HXL_UNEXPECTED_TOKEN,
                        "[Line 1, Col 0] Unexpected token: a",
                        std::get<Error>(Parser::parse(std::get<std::vector<Token>>(tokens))));
        });
    }

    /**
     * NODE.005
     *
     * Whitespace between key and ``:``
     */
    void node005_WhitespaceBetweenKeyAndColon() {
        it("There should not be whitespace between key and colon", [&]() {
            auto tokens = Tokenizer::tokenize("<NodeType> A\n\tkey : B\n");
            assertError(ErrorCode::HXL_UNEXPECTED_TOKEN,
                        "[Line 2, Col 5] Unexpected token: T_WHITESPACE",
                        std::get<Error>(Parser::parse(std::get<std::vector<Token>>(tokens))));
        });
    }

    /**
     * NODE.006
     *
     * Whitespace after ``:`` when declaring property
     */
    void node006_WhitespaceAfterKeyAndColon() {
        it("There should be whitespace after the colon when declaring property", [&]() {
            auto tokens = Tokenizer::tokenize("<NodeType> A\n\tkey:Hello\n");
            assertError(ErrorCode::HXL_UNEXPECTED_TOKEN,
                        "[Line 2, Col 6] Unexpected token: Hello",
                        std::get<Error>(Parser::parse(std::get<std::vector<Token>>(tokens))));
        });
    }

    /**
     * NODE.015
     *
     * Whitespace after property value
     */
    void node015_WhitespaceAfterValue() {
        it("There should not be whitespace after property value.", [&]() {
            auto tokens = Tokenizer::tokenize("<Node> A\n\tkey:Hello{\n");
            assertError(ErrorCode::HXL_UNEXPECTED_TOKEN,
                        "[Line 2, Col 10] Unexpected token: {",
                        std::get<Error>(Parser::parse(std::get<std::vector<Token>>(tokens))));
        });
    }
};
