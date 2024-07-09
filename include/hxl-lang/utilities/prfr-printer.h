#pragma once

#include "hxl-lang/core.h"

namespace HXL::Utilities {
    /**
     * The built-in printer of performance results.
     */
    class PerformanceResultsPrinter {
    public:
        /**
         * Print the set of results.
         *
         * @param results
         */
        static void print(const PerformanceResults &results);
    };
}
