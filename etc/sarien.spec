Name: sarien
Version: 0.6.0
Release: 1cm
Summary: An interpreter for old Sierra AGI games
Group: Games
License: GPL
Source: sarien-0.6.0.tar.gz
URL: http://sarien.sourceforge.net/
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description 
Sarien is an open source, portable implementation of the Sierra On-Line
Adventure Game Interpreter (AGI). It is currently under development;
no production-quality packages have been released.

%prep
%setup -q -n %{name}-%{version}

%build
%configure
make

%install
mkdir -p %{buildroot}%{_localstatedir}/lib/%{name}/
install -D -m755 bin/sarien %{buildroot}/%{_bindir}/sarien
install -D -m644 etc/sarien.cfg %{buildroot}%{_sysconfdir}/sarien.conf

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc doc/README.agi doc/README.unix doc/AUTHORS doc/COPYING doc/BUGS
%doc doc/Changelog
%{_bindir}/sarien
%{_sysconfdir}/sarien.conf

%changelog
* Mon Jun 26 2001 Claudio Matsuoka <claudio@helllabs.org>
- rpm package created

