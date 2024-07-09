using namespace HXL;

class BaseCase : public BBUnit::TestCase {
protected:
    /**
     * Short-hand for asserting an error has occurred, for instance
     * from the Semantic Analyzer or Schema Validator.
     *
     * @param expectedErrorCode
     * @param expectedMessage
     * @param error
     */
    void assertError(ErrorCode expectedErrorCode,
                     const std::string &expectedMessage,
                     const Error &error) {
        assertEquals<ErrorCode>(expectedErrorCode, error.errorCode);
        assertEquals(expectedMessage, error.message);
    }

};