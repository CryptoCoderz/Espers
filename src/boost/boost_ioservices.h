#ifndef BOOST_IOSERVICES_H
#define BOOST_IOSERVICES_H

#include <boost/asio.hpp>

// Boost Support for 1.70+ (Updated)
// Thank you https://github.com/g1itch
#if BOOST_VERSION >= 107000
    #define GetIOService(s) ((boost::asio::io_context&)(s).get_executor().context())
    #define GetIOServiceFromPtr(s) ((boost::asio::io_context&)(s->get_executor().context())) // this one
#else
    #define GetIOService(s) ((s).get_io_service())
    #define GetIOServiceFromPtr(s) ((s)->get_io_service())
#endif

#endif // BOOST_IOSERVICES_H
