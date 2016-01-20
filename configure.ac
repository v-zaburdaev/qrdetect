ACPREREQ([2.68])
AC_INIT([qrdecode],
        [0.0.1],
        [v.zaburdaev@gmail.com],
        [qrdecode])

AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror])
#AC_DEFINE([NDEBUG], [], [Disable debugging information])

AC_LANG([C++])

AC_MSG_CHECKING(checking for OpenCV.)
CPPFLAGS="$CPPFLAGS -D__STDC_CONSTANT_MACROS -I/usr/include/libavformat   -I/usr/local/include/opencv -I/usr/local/include -L/usr/local/lib -I/usr/include "
LIBS="$LIBS -lopencv_highgui -lopencv_core -lopencv_imgproc -lopencv_objdetect -lopencv_video -lopencv_features2d -lopencv_nonfree -lopencv_legacy -lopencv_legacy -lopencv_calib3d -lopencv_flann  -lm -ldecodeqr -lavcodec -lavformat -lavutil"
AC_CHECK_HEADERS([opencv2/core/core_c.h], [], [AC_MSG_ERROR("openCV header missing")],[])

# Checks for libraries.
AC_CHECK_LIB(opencv_core,cvCreateImage,LIBCV=-lopencv_core,)
if test -z "$LIBCV"; then
echo "OpenCV libraries(libcv) not found."
echo 'But your system may already have OpenCV but not in the default("/usr/lib") path'
echo "this script searches. Thus specify the place of the libcv.la as either"
echo "1. In your LDFLAGS environment variables. eg,"
echo 'LDFLAGS="$LDFLAGS -L/usr/local/opencv/lib";export LDFLAGS'
echo '2. As ./configure CPPFLAGS="-I /usr/lib/opencv-1.0.0/include/opencv -L /usr/lib/opencv-1.0.0/lib/"' 
AC_MSG_ERROR(libcv.la not found.)
fi

AC_CHECK_LIB(opencv_highgui,cvShowImage,LIBHIGHGUI=-lopencv_highgui,)
if test -z "$LIBHIGHGUI"; then
echo "OpenCV libraries(libhighgui) not found."
echo 'But your system may already have OpenCV but not in the default("/usr/lib") path'
echo "this script searches. Thus specify the place of the libhighgui.la as either"
echo "1. In your LDFLAGS environment variables. eg,"
echo 'LDFLAGS="$LDFLAGS -L/usr/local/opencv/lib";export LDFLAGS'
echo '2. As ./configure CPPFLAGS="-I /usr/lib/opencv-1.0.0/include/opencv -L /usr/lib/opencv-1.0.0/lib/"' 
AC_MSG_ERROR(libhighgui.la not found.)
fi

#AM_PROG_CC_C_O
AC_PROG_CXX
AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT
