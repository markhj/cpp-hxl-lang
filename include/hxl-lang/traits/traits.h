#pragma once

#include "hxl-lang/core.h"

#include <chrono>
#include <format>

namespace HXL::Traits {
    /**
     * Trait that provides methods to ease/simplify error handling.
     */
    class HandlesErrors {
    protected:
        /**
         * Throw a (streamlined) "Unexpected token" error.
         *
         * @param token
         * @return
         */
        static Error unexpectedTokenError(const Token &token);
    };

    /**
     * Trait that helps measuring execution time
     */
    class MeasuresExecutionTime {
    protected:
        /**
         * Measure the time it takes to execute ``subject``.
         * The result is returned in the ``ExecutionTime`` struct.
         *
         * @param subject
         * @return
         */
        static ExecutionTime measure(const std::function<void()>& subject);

    };
}
