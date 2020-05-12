#include "server.h"
#include "session.h"
#include "log_helper.h"
#include "connection.h"
#include "response.h"
#include <stdexcept>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

server::server(boost::asio::io_service& io_service, short port, log_helper* log, const NginxConfig &config)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      signals_(io_service),
      socket_(tcp::socket(io_service)),
      io_(io_service),
      log_(log),
      config_(config)
{
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
    signals_.async_wait(boost::bind(&server::signal_handler, this,
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::signal_number));
    std::vector<location_parse_result> location_results = config.get_location_result();
    for (location_parse_result loc_res : location_results) {
        if (loc_res.handler_name == "EchoHandler") {  // Location for echo
            std::unique_ptr<request_handler> er(std::move(echo_request_handler::init(config_)));
            location_handlers_.insert(
                std::pair<std::string, std::unique_ptr<request_handler>>(
                    loc_res.uri, std::move(er)));
        } else if (loc_res.handler_name == "StaticHandler") {  // Location for serving static files
            std::unique_ptr<request_handler> sr(std::move(static_request_handler::init(config_)));
            location_handlers_.insert(
                std::pair<std::string, std::unique_ptr<request_handler>>(
                    loc_res.uri, std::move(sr)));
        } else if (loc_res.handler_name == "StatusHandler") {  // Location for listing status info
            std::unique_ptr<request_handler> sr(std::move(status_request_handler::init(config_)));
            location_handlers_.insert(
                std::pair<std::string, std::unique_ptr<request_handler>>(
                    loc_res.uri, std::move(sr)));
        }
    }
    start_accept();
}

void server::signal_handler(const boost::system::error_code& ec,
                            int signal_number)
{
    log_->log_warning_file("server is shutting down");
    acceptor_.close();
    io_.stop();
    throw std::runtime_error("Server Killed");  // Exit to main
}

void server::start_accept()
{
    std::unique_ptr<tcp_connection> conn =
        std::make_unique<tcp_connection>(std::move(socket_));
    std::shared_ptr<session> new_session =
        std::make_shared<session>(std::move(conn), log_, config_, location_handlers_);

    acceptor_.async_accept(
        *(new_session->socket()),
        boost::bind(&server::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
}

void server::handle_accept(std::shared_ptr<session> new_session,
                           const boost::system::error_code& error)
{
    if (!error) {
        new_session->start();
    }
    start_accept();
}
