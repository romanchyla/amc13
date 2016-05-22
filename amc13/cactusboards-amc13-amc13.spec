#
# spefile for AMC13 Library
#
Name: %{name} 
Version: %{version} 
Release: %{release} 
Packager: %{packager}
Summary: AMC13 Library
License: BSD License
Group: CACTUS
Source: https://svnweb.cern.ch/trac/cactus/browser/trunk/cactusboards/amc13/software/amc13
URL: https://svnweb.cern.ch/trac/cactus 
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot 
Prefix: %{_prefix}

%define requires_uhal_version() cactuscore-uhal-uhal >= 2.4.0, cactuscore-uhal-uhal < 2.5.0
BuildRequires: %requires_uhal_version
Requires: %requires_uhal_version


%description
AMC13 Library

%prep

%build


%install 

# copy files to RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_prefix}
cp -rp %{sources_dir}/* $RPM_BUILD_ROOT%{_prefix}/.

#Change access rights
chmod -R 755 $RPM_BUILD_ROOT%{_prefix}/lib
chmod -R 755 $RPM_BUILD_ROOT%{_prefix}/include

%clean 

%post 

%postun 

%files 
%defattr(-, root, root) 
%{_prefix}/lib/*
%{_prefix}/etc/*
%{_prefix}/include/*

