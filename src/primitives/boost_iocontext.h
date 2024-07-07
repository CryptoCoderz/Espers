#ifndef IOCONTEXT_H
#define IOCONTEXT_H

#include <boost/asio.hpp>

#if BOOST_VERSION >= 107000
    typedef boost::asio::io_context ioContext;
#else
    typedef boost::asio::io_service ioContext;
#endif

#endif // IOCONTEXT_H
