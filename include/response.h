#ifndef WNZA_RESPONSE_H_
#define WNZA_RESPONSE_H_

#include <boost/asio.hpp>
#include <vector>
#include <string>

struct header;

class response
{
    std::string return_code;  // Either "200 OK" or "400 Bad Request"
    std::vector<header> headers;
    std::string body;
    std::string entire_header;

public:
    std::vector<boost::asio::const_buffer> build_response();
    void set_status(std::string status_code);
    void add_header(std::string key, std::string val);
    void add_data(std::string data);
    void make_400_error();
    void make_404_error();
};

#endif  // WNZA_RESPONSE_H_
