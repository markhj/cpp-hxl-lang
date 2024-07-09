using namespace HXL;

class SchemaValidatorTest : public BaseCase {
public:
    void test() override {
        arrayValues();
        schema500_InvalidNodeType();
        schema520_RequiredProperty();
        schema522_UnknownProperty();
    }

    /**
     * Check basic schema validation of array values, such as
     * whether the correct value structure has been applied when
     * an array is expected, and vice-versa that a single value
     * structure is applied when not.
     */
    void arrayValues() {
        Schema schema{
                .types = {
                        SchemaNodeType{
                                .name = "A",
                                .properties = {
                                        SchemaNodeProperty{.name = "arr", .dataType = DataType::Int, .structure = ValueStructure::Array},
                                        SchemaNodeProperty{.name = "single", .dataType = DataType::Int},
                                }
                        },
                },
        };

        it("Array structure and single value structure pass", [&]() {
            Result<std::vector<Token>> tokens = Tokenizer::tokenize("<A> A\n\tarr[]: { 1, 2, 3 }\n\tsingle: 10\n");
            Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));
            assertFalse(syntaxTree.isErr());
            std::shared_ptr<Document> document = std::make_shared<Document>(syntaxTree.get());
            std::vector<Error> errors = SchemaValidator::validate(schema, document);

            assertCount(0, errors);
        });

        it("Single value defined as array", [&]() {
            Result<std::vector<Token>> tokens = Tokenizer::tokenize("<A> A\n\tsingle[]: { 1, 2, 3 }\n");
            Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));
            std::shared_ptr<Document> document = std::make_shared<Document>(syntaxTree.get());
            std::vector<Error> errors = SchemaValidator::validate(schema, document);

            assertCount(1, errors);
            assertEquals<std::string>("Property not declared as array: single", errors[0].message);
        });
    }

    /**
     * SCHEMA.500: Invalid node type
     */
    void schema500_InvalidNodeType() {
        it("Checks that there's an unrecognized node type", [&]() {
            Result<std::vector<Token>> tokens = Tokenizer::tokenize("<Sphere> Sphere\n");
            Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));

            Schema schema{
                    .types = {
                            SchemaNodeType{
                                    .name = "Cube",
                            },
                    },
            };

            std::shared_ptr<Document> document = std::make_shared<Document>(syntaxTree.get());
            std::vector<Error> errors = SchemaValidator::validate(schema, document);

            assertCount(1, errors);
            assertEquals<std::string>("Node type not declared in schema: Sphere", errors[0].message);
        });
    }

    /**
     * SCHEMA.520: Required property not found
     */
    void schema520_RequiredProperty() {
    }

    /**
     * SCHEMA.522: Unrecognized property
     */
    void schema522_UnknownProperty() {
    }
};
