using namespace HXL;

class TransformerTest : public BaseCase {
public:
    /**
     * List of tests.
     */
    void test() override {
        inheritance();
    }

    /**
     * Check that a child node can inherit properties from a parent node.
     *
     * However, it shall only inherit properties which are NOT specified.
     *
     * @todo: This code can be drastically simplified.
     */
    void inheritance() {
        it("Adds inherited properties from another node", [&]() {
            std::string inherit = "<Type> A\n\ta: 10\n\tb: 20\n\n<Type> B <= A\n\ta: 15\n";
            Result<std::vector<Token>> tokens = Tokenizer::tokenize(inherit);
            Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));
            std::shared_ptr<Document> document = std::make_shared<Document>(syntaxTree.get());

            Transformer::transform(document);

            uint8_t n = 0;
            assertCount(2, document->nodes);
            assertEquals<std::string>("A", document->nodes[n].name);
            assertEquals<std::string>("a", document->nodes[n].properties[0].name);
            assertEquals<std::string>("10", document->nodes[n].properties[0].values[0]);
            assertEquals<DataType>(DataType::Int, document->nodes[n].properties[0].dataType);
            assertEquals<std::string>("b", document->nodes[n].properties[1].name);
            assertEquals<std::string>("20", document->nodes[n].properties[1].values[0]);
            assertEquals((int) DataType::Int, (int) document->nodes[n].properties[1].dataType);

            ++n;
            assertEquals<std::string>("B", document->nodes[n].name);

            // Value explicitly set by "B", which must NOT be overridden
            assertEquals<std::string>("a", document->nodes[n].properties[0].name);
            assertEquals<std::string>("15", document->nodes[n].properties[0].values[0]);
            assertEquals<DataType>(DataType::Int, document->nodes[n].properties[0].dataType);

            // Value ignored by "B", and which must be inherited from "A"
            assertEquals<std::string>("b", document->nodes[n].properties[1].name);
            assertEquals<std::string>("20", document->nodes[n].properties[1].values[0]);
            assertEquals<DataType>(DataType::Int, document->nodes[n].properties[1].dataType);
        });
    }
};
