Name: sarien
Version: 0.8.0
Release: 1
Summary: An interpreter for old Sierra AGI games
Group: Games
License: GPL
Source: %{name}-%{version}.tar.gz
URL: http://sarien.sourceforge.net/
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description 
Sarien is an open source, portable implementation of the Sierra On-Line
Adventure Game Interpreter (AGI), and is able to run AGI version 2 and
version 3 games such as Space Quest 1 and 2, King's Quest 1 to 4, Gold
Rush! and Leisure Suit Larry in the Land of the Lounge Lizards).

%prep
%setup -q

%build
./configure
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
* Thu Nov 22 2001 Claudio Matsuoka <claudio@helllabs.org>
+ sarien-0.7.0-1cm
- new upstream release

* Sun Aug 26 2001 Claudio Matsuoka <claudio@helllabs.org>
+ sarien-0.6.2-1cm
- new upstream release
 
* Sun Jul 22 2001 Claudio Matsuoka <claudio@helllabs.org>
+ sarien-0.6.1-1cm
- new upstream release
 
* Mon Jun 26 2001 Claudio Matsuoka <claudio@helllabs.org>
+ sarien-0.6.0-1cm
- rpm package created

