#pragma once

#include <string>
#include <vector>

namespace HXL {
    /**
     * A series of helpful functions which can be used through-out the library.
     */
    class Helpers {
    public:
        /**
         * Converts a vector of strings to a vector with "real" types.
         * For instance, a vector of numeric strings can be converted to
         * a vector of integers.
         *
         * The list of data types is intended to match that which
         * can be serialized.
         *
         * @see https://github.com/markhj/hxl-lang/blob/master/specs/hxl-2024/03-data-types.md#array
         *
         * @tparam T
         * @param values
         * @return
         */
        template<typename T>
        static std::vector<T> toArray(const std::vector<std::string> &values) {
            std::vector<T> result;
            for (const std::string &str: values) {
                if constexpr (std::is_same_v<T, int>) {
                    result.push_back(std::stoi(str));
                } else if constexpr (std::is_same_v<T, float>) {
                    result.push_back(std::stof(str));
                } else {
                    result.push_back(str);
                }
            }
            return result;
        }
    };
}
