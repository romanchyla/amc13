
# Library sources 
LibrarySources = $(wildcard src/common/*.cpp)
LibrarySourcesFiltered = $(filter-out ${IgnoreSources}, ${LibrarySources})
LibraryObjectFiles = $(patsubst src/common/%.cpp,obj/%.o,${LibrarySourcesFiltered})

ExecutableSources = $(wildcard src/common/*.cxx)
ExecutableSourcesFiltered = $(filter-out ${IgnoreSources}, ${ExecutableSources})
ExecutableObjectFiles = $(patsubst src/common/%.cxx,obj/%.o,${ExecutableSourcesFiltered})
Executables = $(patsubst src/common/%.cxx,bin/%.exe,${ExecutableSourcesFiltered})

#$(info LibrarySourcesFiltered = ${LibrarySourcesFiltered})
#$(info ExecutableSourcesFiltered = ${ExecutableSourcesFiltered})
#$(info Executables = ${Executables})

# Compiler Flags
IncludePaths = $(addprefix -I,${Includes})

#LinkAllFlags += ${LinkFlags}
#ExecutableLinkAllFlags += ${ExecutableLinkFlags}

# Library dependencies
DependentLibraries += $(addprefix -L,${LibraryPaths})
DependentLibraries += $(addprefix -l,${Libraries})  

# Executable dependencies
ExecutableDependentLibraries += $(addprefix -L,${LibraryPaths})
ExecutableDependentLibraries += $(addprefix -l,${ExecutableLibraries})

ifeq ("${Library}","")
LibraryTarget :=
else
LibraryTarget ?= lib/lib${Library}.so
endif

.PHONY: default
default: build

.PHONY: clean _cleanall
clean: _cleanall
_cleanall:
	rm -rf obj
	rm -rf bin
	rm -f ${LibraryTarget}
	rm -rf lib

.PHONY: all _all build buildall
all: _all
build: _all
buildall: _all
_all: ${LibraryTarget} ${Executables} ${ExtraTargets}



# Implicit rule for .cpp -> .o 
obj/%.o : src/common/%.cpp 
	${MakeDir} {bin,obj,lib}
	${CPP} -c ${CxxFlags} ${IncludePaths} $< -o $@

# Implicit rule for .cxx -> .o 
obj/%.o : src/common/%.cxx 
	${MakeDir} -p {bin,obj,lib}
	${CPP} -c ${CxxFlags} ${IncludePaths} $< -o $@
	
# Main target: shared library
${LibraryTarget}: ${LibraryObjectFiles}
	${LD} ${LinkFlags} ${DependentLibraries} ${LibraryObjectFiles} -o $@

# Include automatically generated dependencies
-include $(LibraryObjectFiles:.o=.d)
	
# Static Pattern rule for binaries
${Executables}: bin/%.exe: obj/%.o ${LibraryTarget}
	${LD} ${ExecutableLinkFlags} ${ExecutableDependentLibraries} $< -o $@

# Include automatically generated dependencies
-include $(ExecutableObjectFiles:.o=.d)

