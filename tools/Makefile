AMC13_ROOT = $(shell pwd)/../

include $(AMC13_ROOT)/config/mfCommonDefs.mk

PackagePath = $(shell pwd)
PackageName = cactusboards-amc13-tools

Packager = Eric Hazen, David Zou, Daniel Gastler

PACKAGE_VER_MAJOR = 1
PACKAGE_VER_MINOR = 1
PACKAGE_VER_PATCH = 10
PACKAGE_RELEASE = 0


LIBRARY = lib/libcactus_amc13_tools.so
LIBRARY_SOURCES = $(wildcard src/common/*.cc)
LIBRARY_OBJECT_FILES = $(patsubst src/common/%.cc,obj/%.o,${LIBRARY_SOURCES})

EXECUTABLE_SOURCES = $(wildcard src/common/*.cxx)
EXECUTABLE_OBJECT_FILES = $(patsubst src/common/%.cxx,obj/%.o,${EXECUTABLE_SOURCES})
EXECUTABLES = $(patsubst src/common/%.cxx,bin/%.exe,${EXECUTABLE_SOURCES})

AMC13_AMC13_PREFIX=../amc13/
AMC13_AMC13_INCLUDE_PREFIX = ${AMC13_AMC13_PREFIX}/include
AMC13_AMC13_LIB_PREFIX = ${AMC13_AMC13_PREFIX}/lib/

INCLUDE_PATH = \
							 -Iinclude  \
							 -I$(AMC13_AMC13_INCLUDE_PREFIX)  \
							 -I$(CACTUS_ROOT)/include 

LIBRARY_PATH = \
							 -Llib \
							 -L$(AMC13_AMC13_LIB_PREFIX) \
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
		-lz

EXECUTABLE_LIBRARIES = ${LIBRARIES} -lcactus_amc13_amc13 -lcactus_amc13_tools

CPP_FLAGS = -g -O3 -rdynamic -Wall -MMD -MP -fPIC ${INCLUDE_PATH}

LINK_LIBRARY_FLAGS = -shared -fPIC -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${LIBRARIES}

LINK_EXECUTABLE_FLAGS = -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${EXECUTABLE_LIBRARIES} 

RPMBUILD_DIR = ${PackagePath}/rpm/RPMBUILD

.PHONY: all _all clean _cleanall build _buildall install _installall rpm _rpmall cleanrpm _cleanrpmall test _testall

default: build
clean: _cleanall
_cleanall:
	rm -rf obj
	rm -rf bin
	rm -rf lib

cleanrpm: _cleanrpmall
_cleanrpmall:
	rm -rf rpm

all: _all
build: _all
buildall: _all
_all: ${LIBRARY} ${EXECUTABLES}

# ------------------------
# Executables
# ------------------------
${EXECUTABLES}: bin/%.exe: obj/%.o ${EXECUTABLE_OBJECT_FILES}
	g++ ${LINK_EXECUTABLE_FLAGS} $< -o $@

${EXECUTABLE_OBJECT_FILES}: obj/%.o : src/common/%.cxx
	mkdir -p {bin,obj,lib}
	g++ -c ${CPP_FLAGS}  $< -o $@

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

rpm: _rpmall
_rpmall: 
	# create the folder structure
	mkdir -p ${RPMBUILD_DIR}/{RPMS/{i386,i586,i686,x86_64},SPECS,BUILD,SOURCES,SRPMS}
	mkdir -p ${RPMBUILD_DIR}/SOURCES/bin/amc13
	
	# copy files over
	find ./bin -name ".svn" -prune -o -exec install -D -m 755 -t ${RPMBUILD_DIR}/SOURCES/bin/amc13 {} \;
	find ./lib -name ".svn" -prune -o -exec install -D -m 755 {} ${RPMBUILD_DIR}/SOURCES/{} \;
	
	# build the rpm
	rpmbuild -bb -bl --buildroot=${RPMBUILD_DIR}/BUILD						\
			--define "_topdir ${RPMBUILD_DIR}"						\
			--define "_prefix ${CACTUS_ROOT}"							\
			--define "sources_dir ${RPMBUILD_DIR}/SOURCES"					\
			--define "name ${PackageName}"								\
			--define "version ${PACKAGE_VER_MAJOR}.${PACKAGE_VER_MINOR}.${PACKAGE_VER_PATCH}"	\
			--define "release ${PACKAGE_RELEASE}.${CACTUS_OS}"							\
			--define "packager ${Packager}"								\
			${PackageName}.spec
	 find  $(PackagePath)/rpm/RPMBUILD -name "*.rpm" -exec mv {} $(PackagePath)/rpm \;

