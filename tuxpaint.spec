Summary: A drawing program for young children
Name: tuxpaint
Version: 0.9.15
Release: 1
License: GPL
Group: Multimedia/Graphics
URL: http://www.newbreedsoftware.com/tuxpaint/
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Requires: SDL >= 1.2.4 SDL_image SDL_mixer SDL_ttf libpng zlib
BuildRequires: SDL-devel >= 1.2.4 SDL_image-devel SDL_mixer-devel SDL_ttf-devel
BuildRequires: libpng-devel zlib-devel gettext

%description
"Tux Paint" is a drawing program for young children.
It provides a simple interface and fixed canvas size,
and provides access to previous images using a thumbnail
browser (e.g., no access to the underlying file-system).

Unlike popular drawing programs like "The GIMP," it has a
very limited tool-set. However, it provides a much simpler
interface, and has entertaining, child-oriented additions
such as sound effects.

%prep
%setup -q
for dir in `find . -name "CVS"`; do
  rm -rf $dir
done

%build
make PREFIX=/usr

%install
rm -rf $RPM_BUILD_ROOT
make PREFIX=/usr PKG_ROOT=$RPM_BUILD_ROOT install

find $RPM_BUILD_ROOT -name tuxpaint.desktop | sort | \
    sed -e "s@$RPM_BUILD_ROOT@@g" > filelist.icons
find $RPM_BUILD_ROOT -name tuxpaint.png | sort | \
    sed -e "s@$RPM_BUILD_ROOT@@g" >> filelist.icons
find $RPM_BUILD_ROOT -name tuxpaint.svg | sort | \
    sed -e "s@$RPM_BUILD_ROOT@@g" >> filelist.icons
find $RPM_BUILD_ROOT -name tuxpaint.xpm | sort | \
    sed -e "s@$RPM_BUILD_ROOT@@g" >> filelist.icons

rm -rf $RPM_BUILD_ROOT/usr/share/doc/tuxpaint


%clean
rm -rf $RPM_BUILD_ROOT

%files -f filelist.icons
%defattr(-,root,root,-)
%doc docs/*

%defattr(0755, root, root)
/usr/bin/*

%defattr(0644, root, root)
/usr/share/locale/*/LC_MESSAGES/tuxpaint.mo
/usr/share/man/man1/*
/usr/share/man/*/man1/tuxpaint.1.gz

%config(noreplace) /etc/tuxpaint/tuxpaint.conf


%defattr(-, root, root)
/usr/share/tuxpaint/*

%changelog
* Thu Sep 15 2005  <shin1@wmail.plala.or.jp> -
- Do not force install desktop icons when Gnome and/or KDE are not installed.

* Sun Mar 27 2005  <shin1@wmail.plala.or.jp> -
- Some hicolor icons not installed were removed from file list

* Fri Jan 14 2005  <bill@newbreedsoftware.com> -
- Changed Group from Amusements/Games to Multimedia/Graphics

* Tue Sep 21 2004  <shin1@wmail.plala.or.jp> -
- Initial build for version 0.9.14
