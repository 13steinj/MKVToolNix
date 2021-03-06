# boost's headers must be present.
AX_BOOST_BASE([1.60.0])

# boost::system must be present.
AX_BOOST_SYSTEM()

# boost::filesystem must be present.
AX_BOOST_FILESYSTEM()

if test x"$ax_cv_boost_filesystem" != "xyes"; then
  AC_MSG_ERROR(The Boost Filesystem Library was not found.)
fi

if test x"$ax_cv_boost_system" != "xyes"; then
  AC_MSG_ERROR(The Boost System Library was not found.)
fi

AX_BOOST_CHECK_HEADERS([boost/rational.hpp],,[
  AC_MSG_ERROR([Boost's rational library is required but wasn't found])
])

AX_BOOST_CHECK_HEADERS([boost/operators.hpp],,[
  AC_MSG_ERROR([Boost's Operators library is required but wasn't found])
])
