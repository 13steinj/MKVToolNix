# boost's headers must be present.
AX_BOOST_BASE([1.49.0])

# boost::system must be present.
AX_BOOST_SYSTEM()

# boost::filesystem must be present.
AX_BOOST_FILESYSTEM()

if test x"$ax_cv_boost_filesystem" != "xyes"; then
  AC_MSG_ERROR(The Boost Filesystem Library was not found.)
fi

# boost::regex must be present.
AX_BOOST_REGEX()

if test x"$ax_cv_boost_regex" != "xyes"; then
  AC_MSG_ERROR(The Boost Regex Library was not found.)
fi

if test x"$ax_cv_boost_system" != "xyes"; then
  AC_MSG_ERROR(The Boost System Library was not found.)
fi

# boost::date_time must be present.
AX_BOOST_DATE_TIME()

if test x"$ax_cv_boost_date_time" != "xyes"; then
  AC_MSG_ERROR(The Boost Date/Time Library was not found.)
fi

AX_BOOST_CHECK_HEADERS([boost/rational.hpp],,[
  AC_MSG_ERROR([Boost's rational library is required but wasn't found])
])

AX_BOOST_CHECK_HEADERS([boost/logic/tribool.hpp],,[
  AC_MSG_ERROR([Boost's Tribool library is required but wasn't found])
])

AX_BOOST_CHECK_HEADERS([boost/lexical_cast.hpp],,[
  AC_MSG_ERROR([Boost's lexical_cast library is required but wasn't found])
])

AX_BOOST_CHECK_HEADERS([boost/math/common_factor.hpp],,[
  AC_MSG_ERROR([Boost's math library is required but its headers weren't found])
])

AX_BOOST_CHECK_HEADERS([boost/range.hpp],,[
  AC_MSG_ERROR([Boost's Range library is required but wasn't found])
])

AX_BOOST_CHECK_HEADERS([boost/variant.hpp],,[
  AC_MSG_ERROR([Boost's variant library is required but wasn't found])
])
