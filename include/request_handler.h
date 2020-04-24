#ifndef WNZA_REQUEST_HANDLER_H_
#define WNZA_REQUEST_HANDLER_H_
#include "request.h"
#include "response.h"

class request_handler
{
public:
    virtual void create_response(const request& req, const std::string& raw_data, response& result) = 0;
    virtual ~request_handler() {};
};
#endif  // WNZA_REQUEST_HANDLER_H_