//
// Created by curtis on 2/18/23.
//

#include "handlers.h"

namespace my_handlers {
    std::string fibonacci(std::string const& query_n) {
        try { // parsing query number
            std::int32_t n = std::stoi(query_n);

            std::stringstream ss;
            ss << "Fibonacci(" << n << "): ";

            if (n <= 1) {
                ss << n << "\r\n";
                return ss.str();
            }
            std::int64_t n2 = 0;
            std::int64_t n1 = 1;
            for (std::int32_t i = 2; i < n; ++i) {
                std::int64_t sum = n1 + n2;
                n2 = n1;
                n1 = sum;
            }
            ss << n2 + n1 << "\r\n";
            return ss.str();
        } catch(std::exception const& e) {
            std::stringstream err("");
            err << "Error: " << e.what() << std::endl;
            return err.str();
        }
    }
}
