AMC13_ROOT = $(shell pwd)/..

include  $(AMC13_ROOT)/config/mfCommonDefs.mk
# CxxFlags := $(filter-out -std=c++0x,$(CxxFlags))
CxxFlags += \
		-ftemplate-depth-128 \
		-O0 -rdynamic -finline-functions \
		-Wno-inline -DNDEBUG

Project = amc13
Package = python

PackagePath = $(AMC13_ROOT)/python
PackageName = cactusboards-amc13-python
PythonModules = ["amc13"]
$(info PythonModules=${PythonModules})

Packager = Alessandro Thea, Tom Williams

PACKAGE_VER_MAJOR = 1
PACKAGE_VER_MINOR = 1
PACKAGE_VER_PATCH = 10
PACKAGE_RELEASE = 0


# The leading undescore is meant to be there
Library = cactus_amc13_python
LibraryTarget = pkg/amc13/_amc13_python.so

Includes = \
    include \
    $(PYTHON_INCLUDE_PREFIX) \
    $(AMC13_ROOT)/amc13/include \
    $(CACTUS_ROOT)/include 

Libraries = \
	boost_python \
	cactus_amc13_amc13 

LibraryPaths = \
	$(AMC13_ROOT)/amc13/lib \
	$(CACTUS_ROOT)/lib 


include ${AMC13_ROOT}/config/mfRules.mk
include ${AMC13_ROOT}/config/mfPythonRPMRules.mk
