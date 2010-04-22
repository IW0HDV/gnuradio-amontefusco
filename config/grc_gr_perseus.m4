dnl Copyright 2001,2002,2003,2004,2005,2006,2008 Free Software Foundation, Inc.
dnl 
dnl This file is part of GNU Radio
dnl 
dnl GNU Radio is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3, or (at your option)
dnl any later version.
dnl 
dnl GNU Radio is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with GNU Radio; see the file COPYING.  If not, write to
dnl the Free Software Foundation, Inc., 51 Franklin Street,
dnl Boston, MA 02110-1301, USA.

AC_DEFUN([GRC_GR_PERSEUS],[
    GRC_ENABLE(gr-perseus)

    dnl Don't do gr-perseus if gnuradio-core skipped
    GRC_CHECK_DEPENDENCY(gr-perseus, gnuradio-core)
    GRC_CHECK_DEPENDENCY(gr-perseus, grc)

    dnl If execution gets to here, $passed will be:
    dnl   with : if the --with code didn't error out
    dnl   yes  : if the --enable code passed muster and all dependencies are met
    dnl   no   : otherwise
    if test $passed = yes; then

        dnl Don't do gr-perseus if the 'libperseus-sdr' library is unavailable.
        AC_CHECK_LIB([perseus-sdr], [perseus_init], 
             [
              echo "Prerequisite for gr-perseus successfully passed configuration checks."
             ],
             [
              passed=no
              echo "Error! To build gr-perseus you need to have libperseus-sdr around."
              echo "Download it from http://code.google.com/p/libperseus-sdr/"
             ])
    fi

    dnl Don't do gr-perseus if gnuradio-core skipped
    GRC_CHECK_DEPENDENCY(gr-perseus, gnuradio-core)

    AC_CONFIG_FILES([ \
        gr-perseus/Makefile \
        gr-perseus/gnuradio-perseus.pc \
        gr-perseus/src/Makefile
    ])

    GRC_BUILD_CONDITIONAL(gr-perseus,[
	dnl run_tests is created from run_tests.in.  Make it executable.
        AC_CONFIG_COMMANDS([run_tests_perseus], [chmod +x gr-perseus/src/run_tests])
    ])
])
