Name:		koviz
Version:	%{_version}
Release:	%{_release}
Prefix:         %{_prefix}
URL:		https://github.com/nasa/koviz
License:	NASA General US Release
Summary:	Trick Sim Plotting
AutoReqProv:	no
Requires:	qt5-qtbase

%description
Koviz is a plotting utility for Trick simulation data

%install
rm -rf %{buildroot}%{prefix}
mkdir -p %{buildroot}%{prefix}
install -m 755 %{name} %{buildroot}%{prefix}

%files
%{prefix}/%{name}
