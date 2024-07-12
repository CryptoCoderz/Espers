#ifndef BOOST_PLACEHOLDERS
#define BOOST_PLACEHOLDERS

#include <boost/asio.hpp>
#include <boost/bind/arg.hpp>
#include <boost/config.hpp>

#if BOOST_VERSION < 107000

namespace boost
{
    namespace placeholders
    {
        static inline boost::arg<1> _1() { return boost::arg<1>(); }
        static inline boost::arg<2> _2() { return boost::arg<2>(); }
        static inline boost::arg<3> _3() { return boost::arg<3>(); }
        static inline boost::arg<4> _4() { return boost::arg<4>(); }
        static inline boost::arg<5> _5() { return boost::arg<5>(); }
        static inline boost::arg<6> _6() { return boost::arg<6>(); }
        static inline boost::arg<7> _7() { return boost::arg<7>(); }
        static inline boost::arg<8> _8() { return boost::arg<8>(); }
        static inline boost::arg<9> _9() { return boost::arg<9>(); }
    } // namespace placeholders
} // namespace boost

#endif // BOOST_VERSION

#endif // BOOST_PLACEHOLDERS
