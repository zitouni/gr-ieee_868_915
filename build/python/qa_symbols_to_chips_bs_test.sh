#!/bin/sh
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/zitouni/gnuradio-3.7.2/gr-ieee_868_915/python
export PATH=/home/zitouni/gnuradio-3.7.2/gr-ieee_868_915/build/python:$PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DYLD_LIBRARY_PATH
export PYTHONPATH=/home/zitouni/gnuradio-3.7.2/gr-ieee_868_915/build/swig:$PYTHONPATH
/usr/bin/python /home/zitouni/gnuradio-3.7.2/gr-ieee_868_915/python/qa_symbols_to_chips_bs.py 
