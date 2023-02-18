#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/url/src.hpp>

#include <boost/asio.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>

#include "handlers.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace urls = boost::urls;
using tcp = boost::asio::ip::tcp;


class http_connection : public std::enable_shared_from_this<http_connection> {
public:
    explicit http_connection(tcp::socket socket)
        : socket_(std::move(socket))
    {}

    void start() {
        read_request();
        check_deadline();
    }

private:
    tcp::socket socket_;

    beast::flat_buffer buffer_{8192};

    http::request<http::dynamic_body> request_;
    http::response<http::dynamic_body> response_;

    net::steady_timer deadline_{
        socket_.get_executor(), std::chrono::seconds(60)
    };

    void read_request() { // poll
        // here to insert trace
        auto self = shared_from_this();

        http::async_read(
            socket_,
            buffer_,
            request_,
            [self](beast::error_code ec, std::size_t bytes_transferred) {
                boost::ignore_unused(bytes_transferred);
                if (!ec)
                    self->process_request();
            });
    }

    void process_request() {
        // StartSpanOptions options;

        response_.version(request_.version());
        response_.keep_alive(false);

        switch(request_.method()) {
            case http::verb::get:
                response_.result(http::status::ok);
                response_.set(http::field::server, "BDML");
                create_response();
                break;

            default: // not implemented yet
                response_.result(http::status::bad_request);
                response_.set(http::field::content_type, "text/plain");
                beast::ostream(response_.body())
                    << "Invalid request method '"
                    << std::string(request_.method_string())
                    << "'";
                break;
        }

        write_response();
    }

    void create_response() { // assume get
        urls::url_view uv(request_.target());
        if (uv.encoded_path() == "/fib") {
            response_.set(http::field::content_type, "text-plain");
            beast::ostream(response_.body())
                << my_handlers::fibonacci((*uv.params().find("n")).value);
        } else {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "Resource Not Fount\r\n";
        }
    }

    void write_response() {
        auto self = shared_from_this();

        response_.content_length(response_.body().size());

        http::async_write(
            socket_,
            response_,
            [self](beast::error_code ec, std::size_t) {
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                self->deadline_.cancel();
            });
    }

    void check_deadline() {
        auto self = shared_from_this();

        deadline_.async_wait(
            [self](beast::error_code ec) {
                if (!ec) {
                    self->socket_.close(ec);
                }
            });
    }
};


void http_server(tcp::acceptor& acceptor, tcp::socket& socket) {
    acceptor.async_accept(socket,
        [&](beast::error_code ec) {
        if (!ec)
            std::make_shared<http_connection>(std::move(socket))->start();
        http_server(acceptor, socket);
    });
}

int main(int argc, char* argv[]) {
    try {
        if(argc != 3) {
            std::cerr << "Usage: " << argv[0] << " <address> <port>\n";
            std::cerr << "Try: 0.0.0.0 80\n";
            return EXIT_FAILURE;
        }

        auto const address = net::ip::make_address(argv[1]);
        auto port = static_cast<unsigned short>(std::atoi(argv[2]));

        net::io_context ioc{1};

        tcp::acceptor acceptor{ioc, {address, port}};
        tcp::socket socket{ioc};
        http_server(acceptor, socket);

        ioc.run();
    } catch(std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}