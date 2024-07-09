#pragma once

#include "hxl-lang/core.h"
#include "hxl-lang/traits/traits.h"

#include <iostream>

namespace HXL {
    /**
     * Shorter form of the struct that is returned by the processor.
     */
    typedef struct {
        PerformanceResults performanceResults;
        std::vector<Error> errors;
    } ProcessResult;

    /**
     * The role of the ``Processor`` is to work as a "full-featured
     * factory" that handles all the necessary steps from tokenizing
     * a source, and up until it has been deserialized.
     */
    class Processor : protected Traits::MeasuresExecutionTime {
    public:
        /**
         * Translate the input source, and validate it through all stages.
         *
         * Last, it will carry out deserialization in accordance with the protocol
         * you have provided.
         *
         * @param source
         * @param schema
         * @param protocol
         * @return
         */
        static ProcessResult process(const std::string &source,
                                     const Schema &schema,
                                     const DeserializationProtocol &protocol);

    };
}
