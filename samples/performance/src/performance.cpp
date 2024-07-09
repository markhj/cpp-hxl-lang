#include "hxl-lang/hxl-lang.h"
#include "hxl-lang/utilities/prfr-printer.h"

// Since debug mode is a lot slower, we will not be processing
// as many there.

#if defined(NDEBUG)
#define SAMPLE_NODES (100 * 1000)
#else
#define SAMPLE_NODES (10 * 1000)
#endif

using namespace HXL;

/**
 * Generate a sample source with thousands of nodes.
 *
 * A couple of different scenarios are covered, such as
 * arrays, references and inheritance.
 *
 * @param nodes
 * @return
 */
std::string generateSource(uint32_t nodes) {
    std::string s;
    for (int i = 0; i < nodes; ++i) {
        switch (i % 5) {
            // Basic
            case 0:
                s += "<A> Node" + std::to_string(i) + "\n";
                s += "\tx: 100";
                s += "\n";
                break;

            // Another basic
            case 1:
                s += "<B> Node" + std::to_string(i) + "\n";
                s += "\tname: \"Hello, World!\"";
                s += "\n";
                break;

            // Reference
            case 2:
                s += "<B> Node" + std::to_string(i) + "\n";
                s += "\tname: \"Hello, World!\"\n";
                s += "\tref&: A";
                s += "\n";
                break;

            // Inheritance
            case 3:
                s += "<B> Node" + std::to_string(i) + " <= Node1\n";
                s += "\tref&: A";
                s += "\n";
                break;

            // Array
            case 4:
                s += "<C> Node" + std::to_string(i) + "\n";
                s += "\tpos[]: { 1, 2, 3 }";
                s += "\n";
                break;

            // Somebody messed up :)
            default:
                assert(false && "Check the switch-case for determining node");
        }
    }
    return s;
}

int main() {
    Schema schema;
    schema.types.push_back({"A", {{"x", DataType::Int}}});
    schema.types.push_back({"B", {
                                         {"name", DataType::String},
                                         {"ref", DataType::NodeRef},
                                 }});
    schema.types.push_back({"C", {{"pos", DataType::Int, ValueStructure::Array}}});

    DeserializationProtocol protocol;
    protocol.handles.push_back({"A", [&](const DeserializedNode &node) {}});
    protocol.handles.push_back({"B", [&](const DeserializedNode &node) {}});
    protocol.handles.push_back({"C", [&](const DeserializedNode &node) {}});

    try {
        // Run the source through the entire translation process
        ProcessResult result = Processor::process(generateSource(SAMPLE_NODES), schema, protocol);

        // Check for errors...
        if (!result.errors.empty()) {
            throw std::runtime_error(result.errors[0].message);
        }

        // Print the performance results
        Utilities::PerformanceResultsPrinter::print(result.performanceResults);

        std::cout << "\nNodes generated: " << SAMPLE_NODES << std::endl;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
