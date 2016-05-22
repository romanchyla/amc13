#!/bin/tcsh -f 
set sourced=($_)
if ("$0" != "tcsh") then
  echo "$0 is meant to be sourced:"
  echo "  source $0"
  exit 0
endif

set CSH_SOURCE=`readlink -f $sourced[2]`

setenv AMC13_STANDALONE_ROOT `dirname $CSH_SOURCE`
setenv CACTUS_ROOT /opt/cactus

setenv PATH "${AMC13_STANDALONE_ROOT}/tools/bin:${PATH}"

setenv LD_LIBRARY_PATH "${CACTUS_ROOT}/lib/:${LD_LIBRARY_PATH}"
setenv LD_LIBRARY_PATH "${AMC13_STANDALONE_ROOT}/amc13/lib/:${LD_LIBRARY_PATH}"
setenv LD_LIBRARY_PATH "${AMC13_STANDALONE_ROOT}/tools/lib/:${LD_LIBRARY_PATH}"
