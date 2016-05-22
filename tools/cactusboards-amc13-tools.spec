#
# spefile for uHAL Library
#
Name: %{name} 
Version: %{version} 
Release: %{release} 
Packager: %{packager}
Summary: AMC13Tool
License: BSD License
Group: CACTUS
Source: https://svnweb.cern.ch/trac/cactus/browser/trunk/cactusupgrades/boards/amc13/software/amc13/tools
URL: https://svnweb.cern.ch/trac/cactus 
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot 
Prefix: %{_prefix}

%description
AMC13Tool Library

%prep

%build

%install 

# copy includes to RPM_BUILD_ROOT and set aliases
mkdir -p $RPM_BUILD_ROOT%{_prefix}
cp -rp %{sources_dir}/* $RPM_BUILD_ROOT%{_prefix}/.

#Change access rights
chmod -R 755 $RPM_BUILD_ROOT%{_prefix}/lib
chmod -R 755 $RPM_BUILD_ROOT%{_prefix}/bin

%clean 

%post 

%postun 

%files 
%defattr(-, root, root) 
%{_prefix}/lib/*
%{_prefix}/bin/*

