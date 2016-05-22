AMC13_ROOT = $(shell pwd)/../

include $(AMC13_ROOT)/config/mfCommonDefs.mk

PackagePath = $(shell pwd)
PackageName = cactusboards-amc13-amc13

Packager = Eric Hazen, David Zou, Daniel Gastler, Dan Arcaro

PACKAGE_VER_MAJOR = 1
PACKAGE_VER_MINOR = 1
PACKAGE_VER_PATCH = 10
PACKAGE_RELEASE = 0


LIBRARY = lib/libcactus_amc13_amc13.so

SOURCES = $(notdir $(wildcard src/common/*.cc))
SOURCE_FILES = $(addprefix src/common/,${SOURCES})
OBJECT_FILES = $(patsubst src/common/%.cc,obj/%.o,${SOURCE_FILES})

# Sources for unpacker code
SOURCES_UNPACKER = $(wildcard src/unpacker/*.cc)
OBJECT_FILES_UNPACKER = $(patsubst src/unpacker/%.cc,obj/%.o,${SOURCES_UNPACKER})

VERSION_SRC = obj/Version.cc
VERSION_OBJ = obj/Version.o


# When compiling standalone
INCLUDE_PATH = 	-Iinclude  \
		-I$(CACTUS_ROOT)/include 

LIBRARY_PATH = \
	-L$(CACTUS_ROOT)/lib 

LIBRARIES =     -lpthread 			\
                -lboost_filesystem 		\
                -lboost_regex 			\
                -lboost_system 			\
                -lboost_thread 			\
                -lcactus_extern_pugixml 	\
                -lcactus_uhal_log 		\
                -lcactus_uhal_grammars 		\
                -lcactus_uhal_uhal


CPP_FLAGS = -g -O3 -rdynamic -Wall -MMD -MP -fPIC ${INCLUDE_PATH} -fno-strict-aliasing

LINK_FLAGS = -g -shared -fPIC -Wall -O3 -rdynamic ${LIBRARY_PATH} ${LIBRARIES}

RPMBUILD_DIR = ${PackagePath}/rpm/RPMBUILD

.PHONY: all _all clean _cleanall build _buildall install _installall rpm _rpmall cleanrpm _cleanrpmall test _testall

default: build
clean: _cleanall
_cleanall:
	rm -rf obj
	rm -rf lib

cleanrpm: _cleanrpmall
_cleanrpmall:
	rm -rf rpm

all: _all
build: _all
buildall: _all
_all: ${LIBRARY}


${LIBRARY}: ${OBJECT_FILES} ${OBJECT_FILES_UNPACKER} 
	@#Build version source file for the AMC13 class
	@echo "#include <amc13/AMC13.hh>" > ${VERSION_SRC}
	@svn -R info | awk 'BEGIN{version=0}{if($$3 == "Rev:"){if($$4 > version){version = $$4}}}END{print "const int amc13::AMC13::Version(" version ");"}' >> ${VERSION_SRC}
	@g++ ${CPP_FLAGS} -c ${VERSION_SRC} -o ${VERSION_OBJ} ; rm ${VERSION_SRC} 
	@#build the actual library
	g++ ${LINK_FLAGS} ${OBJECT_FILES} ${OBJECT_FILES_UNPACKER} ${VERSION_OBJ} -o $@ 
	@#remove any trace of the version file
	@rm ${VERSION_OBJ}


${OBJECT_FILES}: obj/%.o : src/common/%.cc
	mkdir -p {lib,obj}
	g++ ${CPP_FLAGS} -c $< -o $@

${OBJECT_FILES_UNPACKER}: obj/%.o : src/unpacker/%.cc
	mkdir -p {lib,obj}      
	g++ ${CPP_FLAGS} -c $< -o $@

-include $(OBJECT_FILES:.o=.d)
-include $(LIBRARY_OBJECT_FILES_UNPACKER:.o=.d)

rpm: _rpmall
_rpmall: 
	mkdir -p ${RPMBUILD_DIR}/{RPMS/{i386,i586,i686,x86_64},SPECS,BUILD,SOURCES,SRPMS}
	#mkdir -p ${RPMBUILD_DIR}/SOURCES/{lib,include}
	find ./include \( -name "*.[hi]" -o -name "*.hpp" -o -name "*.hh" -o -name "*.hxx" \) -exec install -D -m 644 {} ${RPMBUILD_DIR}/SOURCES/{} \;
	#find ./bin -name ".svn" -prune -o -exec install -D -m 755 {} ${RPMBUILD_DIR}/SOURCES/{} \;
	find ./lib -name ".svn" -prune -o -exec install -D -m 755 {} ${RPMBUILD_DIR}/SOURCES/{} \;
	find ./etc -name ".svn" -prune -o -exec install -D -m 644 {} ${RPMBUILD_DIR}/SOURCES/{} \;
	rpmbuild -bb -bl --buildroot=${RPMBUILD_DIR}/BUILD						\
			--define  "_topdir ${RPMBUILD_DIR}"						\
			--define "_prefix ${CACTUS_ROOT}"							\
			--define "sources_dir ${RPMBUILD_DIR}/SOURCES"					\
			--define "name ${PackageName}"								\
			--define "version ${PACKAGE_VER_MAJOR}.${PACKAGE_VER_MINOR}.${PACKAGE_VER_PATCH}"	\
			--define "release ${PACKAGE_RELEASE}.${CACTUS_OS}"							\
			--define "packager ${Packager}"								\
			${PackageName}.spec
	find  $(PackagePath)/rpm/RPMBUILD -name "*.rpm" -exec mv {} $(PackagePath)/rpm \;


