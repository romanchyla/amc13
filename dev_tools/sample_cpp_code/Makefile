AMC13_ROOT = $(shell pwd)/../..

include $(AMC13_ROOT)/config/mfCommonDefs.mk

LIBRARY = lib/libcactus_amc13_dev_tools.so
LIBRARY_SOURCES = $(wildcard src/common/*.cc)
LIBRARY_OBJECT_FILES = $(patsubst src/common/%.cc,obj/%.o,${LIBRARY_SOURCES})

EXECUTABLE_SOURCES = $(wildcard src/common/*.cxx)
EXECUTABLE_OBJECT_FILES = $(patsubst src/common/%.cxx,obj/%.o,${EXECUTABLE_SOURCES})
EXECUTABLES = $(patsubst src/common/%.cxx,bin/%.exe,${EXECUTABLE_SOURCES})

AMC13_AMC13_PREFIX=${AMC13_ROOT}/amc13/
AMC13_AMC13_INCLUDE_PREFIX = ${AMC13_AMC13_PREFIX}/include
AMC13_AMC13_LIB_PREFIX = ${AMC13_AMC13_PREFIX}/lib/
AMC13_TOOLS_LIB_PREFIX = ${AMC13_ROOT}/tools/lib/

INCLUDE_PATH = \
							 -Iinclude  \
							 -I$(AMC13_ROOT)/tools/include  \
							 -I$(AMC13_AMC13_INCLUDE_PREFIX)  \
							 -I$(CACTUS_ROOT)/include

LIBRARY_PATH = \
							 -Llib \
							 -L$(AMC13_AMC13_LIB_PREFIX) \
							 -L$(AMC13_TOOLS_LIB_PREFIX) \
							 -L$(CACTUS_ROOT)/lib 

LIBRARIES =     -lpthread 			\
                -lboost_filesystem 		\
                -lboost_regex 			\
                -lboost_system 			\
                -lboost_thread 			\
                -lcactus_extern_pugixml 	\
                -lcactus_uhal_log 		\
                -lcactus_uhal_grammars 		\
                -lcactus_uhal_uhal \
								-lreadline \
								-lcurses \

#EXECUTABLE_LIBRARIES = ${LIBRARIES} -lcactus_amc13_amc13 -lcactus_amc13_tools -lcactus_amc13_dev_tools
EXECUTABLE_LIBRARIES = ${LIBRARIES} -lcactus_amc13_amc13 -lcactus_amc13_tools

CPP_FLAGS = -g -O3 -rdynamic -Wall -MMD -MP -fPIC ${INCLUDE_PATH}

LINK_LIBRARY_FLAGS = -shared -fPIC -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${LIBRARIES}
#LINK_LIBRARY_FLAGS = -static -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${LIBRARIES}

LINK_EXECUTABLE_FLAGS = -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${EXECUTABLE_LIBRARIES} 

.PHONY: all _all clean _cleanall build _buildall install _installall test _testall

default: build
clean: _cleanall
_cleanall:
	rm -rf obj
	rm -rf bin
	rm -rf lib

all: _all
build: _all
buildall: _all
_all: ${LIBRARY} ${EXECUTABLES}

# ------------------------
# Executables
# ------------------------
#${EXECUTABLES}: bin/%.exe: obj/%.o ${EXECUTABLE_OBJECT_FILES} ${LIBRARY}
${EXECUTABLES}: bin/%.exe: obj/%.o ${EXECUTABLE_OBJECT_FILES} ${LIBRARY_OBJECT_FILES}
	g++ ${LINK_EXECUTABLE_FLAGS} $< ${LIBRARY_OBJECT_FILES} -o $@

${EXECUTABLE_OBJECT_FILES}: obj/%.o : src/common/%.cxx
	mkdir -p {bin,obj,lib}
	g++ -c ${CPP_FLAGS}   $< -o $@

-include $(EXECUTABLE_OBJECT_FILES:.o=.d)

# ------------------------
# library
# ------------------------
${LIBRARY}: ${LIBRARY_OBJECT_FILES}
	g++ ${LINK_LIBRARY_FLAGS} ${LIBRARY_OBJECT_FILES} -o $@

${LIBRARY_OBJECT_FILES}: obj/%.o : src/common/%.cc 
	mkdir -p {lib,obj}
	g++ ${CPP_FLAGS} -c $< -o $@

-include $(LIBRARY_OBJECT_FILES:.o=.d)
