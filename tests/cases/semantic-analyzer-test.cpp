using namespace HXL;

class SemanticAnalyzerTest : public BaseCase {
public:
    /**
     * List of tests.
     */
    void test() override {
        node200_UniqueNodeName();
        node201_UniquePropertyName();
        ref200_ReferenceMustExist();
        ref201_ReferenceMustBeDeclaredBeforeUse();
        ref203_SelfReferencing();
        inhr201_InheritedNodeMustExist();
        inhr202_InheritedNodesMustBeDeclaredBeforeUse();
        inhr203_InheritSelf();
    }

    /**
     * Helper method which checks that a provided source results in exactly one
     * error, with the given code and message.
     *
     * @param source
     * @param errorCode
     * @param errorMessage
     */
    void assertSemanticError(const std::string &source,
                             ErrorCode errorCode,
                             const std::string &errorMessage) {
        auto tokens = Tokenizer::tokenize(source);
        Result<Document> syntaxTree = Parser::parse(std::get<std::vector<Token>>(tokens));
        std::vector<Error> errors = SemanticAnalyzer::analyze(std::make_shared<Document>(syntaxTree.get()));

        assertCount(1, errors);
        assertEquals<ErrorCode>(errorCode, errors[0].errorCode);
        assertEquals<std::string>(errorMessage, errors[0].message);
    }

    /**
     * Check that node names are unique.
     */
    void node200_UniqueNodeName() {
        it("Checks that node names are unique", [&]() {
            assertSemanticError("<NodeType> A\n\n<NodeType> B\n\n<NodeType> A\n",
                                ErrorCode::HXL_NON_UNIQUE_NODE,
                                "Node name \"A\" is not unique.");
        });
    }

    /**
     * Check that the properties under a node are unique.
     */
    void node201_UniquePropertyName() {
        it("Checks that property names are unique", [&]() {
            assertSemanticError("<NodeType> A\n\ta: 8\n\tb: 10\n\ta: 11\n",
                                ErrorCode::HXL_NON_UNIQUE_PROPERTY,
                                R"(Property "a" under "A" is not unique.)");
        });
    }

    /**
     * REF.200
     *
     * A referenced node must exist
     */
    void ref200_ReferenceMustExist() {
        it("Checks that a node exists when referenced.", [&]() {
            assertSemanticError("<Node> A\n\tref&: B\n",
                                ErrorCode::HXL_NODE_REFERENCE_NOT_FOUND,
                                R"(Referenced node "B" under A:ref was not found.)");
        });
    }

    /**
     * REF.201
     *
     * Node must be declared before being referenced.
     */
    void ref201_ReferenceMustBeDeclaredBeforeUse() {
        it("Checks that a node is declared before being referenced.", [&]() {
            assertSemanticError("<Node> A\n\tref&: B\n<Node> B\n",
                                ErrorCode::HXL_NODE_REFERENCE_NOT_FOUND,
                                R"(Referenced node "B" under A:ref was not found.)");
        });
    }

    /**
     * REF.203
     *
     * A node is not allowed to reference itself.
     */
    void ref203_SelfReferencing() {
        it("Checks that a node cannot reference itself.", [&]() {
            assertSemanticError("<Node> A\n\tref&: A\n",
                                ErrorCode::HXL_ILLEGAL_REFERENCE,
                                R"(A:ref is referencing itself.)");
        });
    }

    /**
     * INHR.201
     *
     * Inherited node must exist.
     */
    void inhr201_InheritedNodeMustExist() {
        it("Checks that an inherited exists.", [&]() {
            assertSemanticError("<Node> A <= B\n",
                                ErrorCode::HXL_ILLEGAL_INHERITANCE,
                                R"(Node A attempts to inherit B which does not exist.)");
        });
    }

    /**
     * INHR.202
     *
     * Inherited node must be declared before being used.
     */
    void inhr202_InheritedNodesMustBeDeclaredBeforeUse() {
        it("Checks that a node has been declared before being inherited.", [&]() {
            assertSemanticError("<Node> A <= B\n\ta: 2\n<Node> B\n",
                                ErrorCode::HXL_ILLEGAL_INHERITANCE,
                                R"(Node A attempts to inherit B which does not exist.)");
        });
    }

    /**
     * INHR.203
     *
     * A node cannot inherit itself.
     */
    void inhr203_InheritSelf() {
        it("Checks that a node cannot inherit itself..", [&]() {
            assertSemanticError("<Node> A <= A\n",
                                ErrorCode::HXL_ILLEGAL_INHERITANCE,
                                R"(Node A cannot inherit itself.)");
        });
    }
};
