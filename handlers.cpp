//
// Created by curtis on 2/18/23.
//

#include "handlers.h"

#include "tracer_common.h"

namespace trace = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;

namespace my_handlers {
    std::string fibonacci(std::string const& query_n) {
        auto span = get_tracer("fib_server")->StartSpan("fibonacci");
        auto scope = opentelemetry::trace::Scope(span);

        span->AddEvent("entering fibonacci");

        try { // parsing query number
            std::int32_t n = std::stoi(query_n);

            span->SetAttribute("query-n", n);
            std::stringstream ss;
            ss << "Fibonacci(" << n << ")*2: ";

            if (n <= 1) {
                auto times2result = times2(n);
                ss << times2result << "\r\n";
                return ss.str();
            }
            std::int64_t n2 = 0;
            std::int64_t n1 = 1;
            for (std::int32_t i = 2; i < n; ++i) {
                std::int64_t sum = n1 + n2;
                n2 = n1;
                n1 = sum;
            }

            span->SetAttribute("fibonacci", n1 + n2);

            auto times2result = times2(n1 + n2);
            ss << times2result << "\r\n";
            return ss.str();
        } catch(std::exception const& e) {
            std::stringstream err("");
            err << "Error: " << e.what() << std::endl;
            span->SetAttribute("fibonacci error", e.what());
            return err.str();
        }
    }

    std::int64_t times2(std::int64_t x) {
        auto span = get_tracer("fib_server")->StartSpan("times2");
        auto scope = opentelemetry::trace::Scope(span);
        auto result = x * 2;

        span->SetAttribute("times2", result);
        span->AddEvent("times2 with fibonacci");
        return result;
    }
}
