using namespace HXL;

class DeserializerTest : public BaseCase {
public:
    /**
     * List of tests for the deserializer
     */
    void test() override {
        basic();
        reference();
        inheritance();
        array();
    }

    /**
     * A basic test of how the deserializer works.
     */
    void basic() {
        it("Deserializes integer values", [&]() {
            struct Cube {
                float size;
            };

            std::vector<Cube> cubes;

            Result<std::vector<Token>> tokens = Tokenizer::tokenize("<Cube> MyCube\n\tsize: 8.0\n<Cube> CubeTwo\n\tsize: 10.0\n");
            Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));

            DeserializationProtocol protocol;
            protocol.handles.push_back({"Cube", [&](const DeserializedNode &node) {
                                            DeserializedNodeProperty size = node.properties.at("size");
                                            cubes.push_back(Cube{std::get<float>(size.value)});
                                        }});

            std::shared_ptr<Document> document = std::make_shared<Document>(syntaxTree.get());
            Deserializer::deserialize(protocol, document);

            assertCount(2, cubes);
            assertEquals<float>(8.0, cubes[0].size);
            assertEquals<float>(10.0, cubes[1].size);
        });
    }

    /**
     * Test that the deserializer provides a NodeRef struct
     * when it translated a node reference.
     */
    void reference() {
        it("Deserializes references", [&]() {
            Result<std::vector<Token>> tokens = Tokenizer::tokenize("<Cube> MyCube\n\n<Cube> CubeTwo\n\tref&: MyCube\n");
            Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));

            std::vector<NodeRef> nodeRefs;

            DeserializationProtocol protocol;
            protocol.handles.push_back({"Cube", [&](const DeserializedNode &node) {
                                            if (node.properties.contains("ref")) {
                                                nodeRefs.push_back(std::get<NodeRef>(node.properties.at("ref").value));
                                            }
                                        }});

            std::shared_ptr<Document> document = std::make_shared<Document>(syntaxTree.get());
            Deserializer::deserialize(protocol, document);

            assertCount(1, nodeRefs);
            assertEquals<std::string>("MyCube", nodeRefs[0].references);
        });
    }

    /**
     * Test when a node inherits another node that it gets
     * the parent node's properties.
     */
    void inheritance() {
        it("Deserializes inherited properties", [&]() {
            struct Cube {
                float size;
            };

            std::vector<Cube> cubes;

            Result<std::vector<Token>> tokens = Tokenizer::tokenize("<Cube> MyCube\n\tsize: 8.0\n<Cube> CubeTwo <= MyCube\n");
            Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));
            std::shared_ptr<Document> document = std::make_shared<Document>(syntaxTree.get());

            // Run the transform so the inherited properties are resolved
            Transformer::transform(document);

            DeserializationProtocol protocol;
            protocol.handles.push_back({"Cube", [&](const DeserializedNode &node) {
                                            DeserializedNodeProperty size = node.properties.at("size");
                                            cubes.push_back(Cube{std::get<float>(size.value)});
                                        }});

            Deserializer::deserialize(protocol, document);

            assertCount(2, cubes);
            assertEquals<float>(8.0, cubes[0].size);
            assertEquals<float>(8.0, cubes[1].size);
        });
    }

    /**
     * Helper function to test that the contents of an array
     * are deserialized to the correct data type, and with
     * the correct values.
     *
     * @tparam T
     * @param arrValues
     * @param expected
     */
    template<typename T>
    void testArray(const std::string &arrValues,
                   const std::array<T, 3> &expected) {
        Result<std::vector<Token>> tokens = Tokenizer::tokenize("<Cube> MyCube\n\tpos[]: { " + arrValues + " }\n");
        Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));
        std::vector<T> values;

        DeserializationProtocol protocol;
        protocol.handles.push_back({"Cube", [&](const DeserializedNode &node) {
                                        auto v = std::get<std::vector<T>>(node.properties.at("pos").value);
                                        values.insert(values.end(), v.begin(), v.end());
                                    }});

        std::shared_ptr<Document> document = std::make_shared<Document>(syntaxTree.get());
        Deserializer::deserialize(protocol, document);

        assertCount(3, values);
        assertEquals<T>(expected[0], values[0]);
        assertEquals<T>(expected[1], values[1]);
        assertEquals<T>(expected[2], values[2]);
    }

    /**
     * Test deserialization of the supported array types.
     */
    void array() {
        it("Deserializes an array", [&]() {
            testArray<float>("1.5, 6.0, -3.0", {1.5, 6.0, -3.0});
            testArray<int>("-3, 0, 3", {-3, 0, 3});
            testArray<std::string>(R"("Hello", "World", "!")", {"Hello", "World", "!"});
        });
    }
};
