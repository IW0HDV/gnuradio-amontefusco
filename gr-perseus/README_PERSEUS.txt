PERSEUS on GNU Radio
--------------------

To successfully build gr-perseus module, you have to install the libperseus-sdr 
library as found in

http://code.google.com/p/libperseus-sdr/

cd ~
svn checkout http://libperseus-sdr.googlecode.com/svn/tags/REL-0.2 libperseus-sdr
cd libperseus-sdr

Next follow the instructions in README to build and install the library.



------------------------------------------------------
cd ~/gnuradio/gr-perseus
.
./gnuradio-perseus.pc.in
./apps
./apps/qa_perseus_wfm.py
./apps/qa_perseus_am.py
./apps/qa_sr_amrx.py
./apps/qa_audio.py
./apps/qa_perseus_rx.py
./Makefile.am
./README_PERSEUS.txt
./src
./src/perseus_source_c.h
./src/perseus_source_c.cc
./src/Makefile.swig.gen
./src/run_tests.in
./src/Makefile.am
./src/qa_perseus.py
./src/perseus.i

