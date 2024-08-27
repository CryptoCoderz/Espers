#ifndef BOOST_PLACEHOLDERS
#define BOOST_PLACEHOLDERS

#include <boost/asio.hpp>
#include <boost/bind/arg.hpp>
#include <boost/config.hpp>

// Legacy Boost Version handling
#if BOOST_VERSION < 107000

#include <boost/bind.hpp>

#else

// Modern Boost Version hanlding
#include <boost/bind/bind.hpp>
using namespace boost::placeholders;

#endif // BOOST_VERSION
#endif // BOOST_PLACEHOLDERS
