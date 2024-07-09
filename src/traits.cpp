#include "hxl-lang/traits/traits.h"

HXL::Error HXL::Traits::HandlesErrors::unexpectedTokenError(const Token &token) {
    return {
            .errorCode = ErrorCode::HXL_UNEXPECTED_TOKEN,
            .message = std::format("[Line {}, Col {}] Unexpected token: {}",
                                   token.position.line,
                                   token.position.col,
                                   token.toString()),
    };
}

HXL::ExecutionTime HXL::Traits::MeasuresExecutionTime::measure(const std::function<void()>& subject) {
    auto start = std::chrono::high_resolution_clock::now();

    subject();

    auto end = std::chrono::high_resolution_clock::now();

    return {
            .ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(),
    };
}
