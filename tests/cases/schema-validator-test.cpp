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
                                }},
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
     * Helper method to assert a specific schema rule.
     *
     * @param source
     * @param errorCode
     * @param errorMessage
     */
    void assertSchemaRule(const std::string &source,
                          ErrorCode errorCode,
                          const std::string &errorMessage) {
        Result<std::vector<Token>> tokens = Tokenizer::tokenize(source);
        Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));

        Schema schema{
                .types = {
                        SchemaNodeType{
                                .name = "Sphere",
                                .properties = {
                                        SchemaNodeProperty{.name = "required",
                                                           .dataType = DataType::Int,
                                                           .required = true},
                                },
                        },
                },
        };

        std::shared_ptr<Document> document = std::make_shared<Document>(syntaxTree.get());
        std::vector<Error> errors = SchemaValidator::validate(schema, document);

        assertCount(1, errors);
        assertEquals<ErrorCode>(errorCode, errors[0].errorCode);
        assertEquals<std::string>(errorMessage, errors[0].message);
    }

    /**
     * SCHEMA.500: Invalid node type
     */
    void schema500_InvalidNodeType() {
        it("Checks that there's an unrecognized node type", [&]() {
            assertSchemaRule("<Cube> A\n",
                             ErrorCode::HXL_UNKNOWN_NODE_TYPE,
                             "Node type not declared in schema: Cube");
        });
    }

    /**
     * SCHEMA.520: Required property not found
     */
    void schema520_RequiredProperty() {
        it("Checks that a required property is not present.", [&]() {
            assertSchemaRule("<Sphere> A\n",
                             ErrorCode::HXL_REQUIRED_PROPERTY_NOT_FOUND,
                             "Node A is missing required property: required");
        });
    }

    /**
     * SCHEMA.522: Unrecognized property
     */
    void schema522_UnknownProperty() {
        it("Checks that all defined properties must be defined in schema.", [&]() {
            assertSchemaRule("<Sphere> A\n\tunknown: 10\n\trequired: 10\n",
                             ErrorCode::HXL_UNKNOWN_PROPERTY,
                             "Node A has an unknown property: unknown");
        });
    }
};
