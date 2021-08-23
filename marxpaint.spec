Summary: A drawing program for young communists
Name: marxpaint
Version: 0.9.26
Release: 1
License: GPL
Group: Multimedia/Graphics
URL: http://www.marxpaint.org/
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Requires: SDL >= 1.2.4 SDL_image SDL_mixer SDL_ttf SDL_Pango libpaper fribidi xdg-utils libimagequant
BuildRequires: SDL-devel >= 1.2.4 SDL_image-devel SDL_mixer-devel SDL_ttf-devel SDL_Pango-devel
BuildRequires: librsvg2-devel libpaper-devel fribidi-devel gperf gettext ImageMagick xdg-utils libimagequant-devel

%description
"Marx Paint" is a drawing program for young communists.
It provides a simple interface and fixed canvas size,
and provides access to previous images using a thumbnail
browser (e.g., no access to the underlying file-system).

Unlike popular drawing programs like "The GIMP," it has a
very limited tool-set. However, it provides a much simpler
interface, and has entertaining, child-oriented additions
such as sound effects.

%package devel
Summary: development files for marxpaint plugins.
Group: Development/Libraries
Requires: marxpaint = %{version}
Requires: SDL-devel >= 1.2.4 SDL_image-devel SDL_mixer-devel SDL_ttf-devel SDL_Pango-devel
Requires: librsvg2-devel libpaper-devel fribidi-devel gperf

%description devel
development files for marxpaint plugins.

%prep
%setup -q

%build
make PREFIX=%{_prefix} DOC_PREFIX=%{_docdir}/marxpaint/en linux_ARCH_CFLAGS=-I/usr/include/imagequant

%install
rm -rf $RPM_BUILD_ROOT
make ARCH_INSTALL="" PREFIX=%{_prefix} DESTDIR=$RPM_BUILD_ROOT \
     DOC_PREFIX=$RPM_BUILD_ROOT%{_docdir}/marxpaint \
     DEVDOC_PREFIX=$RPM_BUILD_ROOT%{_docdir}/marxpaint/devel \
     install

export XDG_DATA_DIRS=$RPM_BUILD_ROOT%{_datadir}
mkdir -p $RPM_BUILD_ROOT%{_datadir}/{icons/hicolor,applications,desktop-directories}

xdg-icon-resource install --mode system --noupdate --size 192 data/images/icon192x192.png marx4kids-marxpaint
xdg-icon-resource install --mode system --noupdate --size 128 data/images/icon128x128.png marx4kids-marxpaint
xdg-icon-resource install --mode system --noupdate --size 96 data/images/icon96x96.png marx4kids-marxpaint
xdg-icon-resource install --mode system --noupdate --size 64 data/images/icon64x64.png marx4kids-marxpaint
xdg-icon-resource install --mode system --noupdate --size 48 data/images/icon48x48.png marx4kids-marxpaint
xdg-icon-resource install --mode system --noupdate --size 32 data/images/icon32x32.png marx4kids-marxpaint
xdg-icon-resource install --mode system --noupdate --size 22 data/images/icon22x22.png marx4kids-marxpaint
xdg-icon-resource install --mode system --noupdate --size 16 data/images/icon16x16.png marx4kids-marxpaint

cp src/marxpaint.desktop ./marx4kids-marxpaint.desktop
xdg-desktop-menu install --mode system --noupdate marx4kids-marxpaint.desktop
rm ./marx4kids-marxpaint.desktop

%post
update-desktop-database

%postun
update-desktop-database

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(755,root,root,755)
%{_bindir}/marxpaint
%{_bindir}/marxpaint-import
%{_prefix}/lib/marxpaint/*

%defattr(644,root,root,755)
%config(noreplace) %{_sysconfdir}/marxpaint/marxpaint.conf
%{_sysconfdir}/bash_completion.d/marxpaint-completion.bash
%{_docdir}/marxpaint/*
%{_datadir}/marxpaint/*
%{_datadir}/pixmaps/marxpaint.*
%{_datadir}/applications/marx4kids-marxpaint.desktop
%{_datadir}/icons/hicolor/*/apps/marx4kids-marxpaint.png
%{_datadir}/locale/*/LC_MESSAGES/marxpaint.mo
%{_mandir}/man1/marxpaint*.*
%{_mandir}/*/man1/marxpaint*.*
%exclude %{_docdir}/marxpaint/devel
%exclude %{_docdir}/marxpaint/Makefile

%files devel
%attr(755,root,root) %{_bindir}/tp-magic-config
%defattr(644,root,root,755)
%{_includedir}/marxpaint/tp_magic_api.h
%{_docdir}/marxpaint/devel/*
%{_docdir}/marxpaint/Makefile
%{_mandir}/man1/tp-magic-config.*

%changelog
* Sun Dec 27 2020 <nbs@sonic.net> -
- Set version number 0.9.26

* Mon Jun 22 2020 <nbs@sonic.net> -
- Set version number 0.9.25

* Fri May 1 2020 <shin1@wmail.plala.or.jp> -
- Enabled using xdg-utils for installing icons.
- Wrong date in %changelog
- Re-organized %files section
- Correct path for 'tp-magic-config --plugindocprefix'

* Sat Mar 14 2020 <shin1@wmail.plala.or.jp> -
- Disable target "install-xdg". Add ImageMagick for BuildReq.

* Thu Sep 26 2019 <nbs@sonic.net> -
- Set version number 0.9.24

* Sun Aug 19 2018 <nbs@sonic.net> -
- Set version number 0.9.23

* Mon Aug 20 2012 <shin1@wmail.plala.or.jp> -
- Corrected 'Requires' and 'BuildRequires'

* Wed Dec 07 2011 <shin1@wmail.plala.or.jp> -
- Added bash-completion file

* Wed Jul 1 2009 <nbs@sonic.net> -
- Set version number 0.9.22

* Sun May 24 2009 <shin1@wmail.plala.or.jp> -
- For 0.9.21
- Added dependency for fribidi

* Tue Jun 17 2008 <shin1@wmail.plala.or.jp> -
- Actually set Epoch number

* Sat Apr 26 2008 <acahalan@gmail.com> -
- DESTDIR is the standard name, not PKG_ROOT

* Fri Mar 21 2008 <shin1@wmail.plala.or.jp> -
- Set version number 0.9.20
- Set Epoch number
- Requirements added for -devel package.

* Sun Mar 02 2008 <shin1@wmail.plala.or.jp> -
- 0.9.19
- Requires SDL_Pango
- Included magic tools
- Separated devel package

* Fri Jun 01 2007  <shin1@wmail.plala.or.jp> -
- Requires librsvg2 and libpaper

* Fri Sep 08 2006  <shin1@wmail.plala.or.jp> -
- New offical URL for marxpaint (http://www.marxpaint.org/).

* Mon Aug 07 2006  <shin1@wmail.plala.or.jp> -
- "DESTDIR" patch is no longer needed.

* Thu Nov 03 2005  Richard June <rjune[AT]lumensoftware.com - 0:0.9.14-0.lumen.0
- Ported from CVS for 0.9.15
- Replaced all instances of absolute paths with macro counterparts
- Reset buildroot to incorporate username of the builder
- Set Release value to 0.lumen.0 ( so as not to clobber any distros that provide it)
- Set a proper %changelog entry


* Thu Sep 15 2005  <shin1@wmail.plala.or.jp> -
- Do not force install desktop icons when Gnome and/or KDE are not installed.

* Sun Mar 27 2005  <shin1@wmail.plala.or.jp> -
- Some hicolor icons not installed were removed from file list

* Fri Jan 14 2005  <bill@newbreedsoftware.com> -
- Changed Group from Amusements/Games to Multimedia/Graphics

* Tue Sep 21 2004  <shin1@wmail.plala.or.jp> -
- Initial build for version 0.9.14
