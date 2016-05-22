%define _package __package__
%define _packagename __packagename__
%define _version __version__
%define _release __release__
%define _prefix  __prefix__
%define _sources_dir __sources_dir__
%define _tmppath /tmp
%define _packagedir __packagedir__
%define _os __os__
%define _platform __platform__
%define _project __project__
%define _author __author__
%define _summary __summary__
%define _url __url__
#%define _buildarch __buildarch__
#%define _includedirs __includedirs__

#
# SWATCH Specfile template
#
Name: %{_packagename} 
Version: %{_version} 
Release: %{_release} 
Packager: %{_author}
Summary: %{_summary}
License: BSD License
Group: CACTUS
Source: %{_source}
URL: %{_url} 
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot 
Prefix: %{_prefix}

# TODO: include requirements
# References: 
# cmsos template file: https://svnweb.cern.ch/trac/cmsos/browser/trunk/config/spec.template
# cmsos RPM rules: https://svnweb.cern.ch/trac/cmsos/browser/trunk/config/mfRPM.rules
# Notice the _require and __require__ interplay with PACKAGE_REQUIRED_PACKAGE_LIST
# AMC13 example
#%define requires_uhal_version() cactuscore-uhal-uhal >= 2.3.0, cactuscore-uhal-uhal < 2.4.0
#BuildRequires: %requires_uhal_version
#Requires: %requires_uhal_version


%description
__description__

%prep

%build

%install 
# copy files to RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_prefix}/{bin,lib,include,etc}

if [ -d %{_packagedir}/bin ]; then
  cd %{_packagedir}/bin; \
  find . -name "*"  -exec install -D -m 755 {} $RPM_BUILD_ROOT/%{_prefix}/bin/{} \;
fi

if [ -d %{_packagedir}/include ]; then
  cd %{_packagedir}/include; \
  find . \( -name "*.hpp"  -o -name "*.hxx" \)  -exec install -D -m 644 {} $RPM_BUILD_ROOT/%{_prefix}/include/{} \;
fi

if [ -d %{_packagedir}/lib ]; then
  cd %{_packagedir}/lib; \
  find . -name "*" -exec install -D -m 644 {} $RPM_BUILD_ROOT/%{_prefix}/lib/{} \;
fi

if [ -d %{_packagedir}/etc ]; then
  cd %{_packagedir}/etc; \
  find . -name "*" -exec install -D -m 644 {} $RPM_BUILD_ROOT/%{_prefix}/etc/{} \;
fi

#cp -rp %{_sources_dir}/* $RPM_BUILD_ROOT%{_prefix}/.


%clean 

%post 

%postun 

%files 
%defattr(-, root, root) 
%{_prefix}/bin
%{_prefix}/lib
%{_prefix}/etc
%{_prefix}/include

