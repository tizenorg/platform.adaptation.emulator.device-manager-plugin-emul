Name:       device-manager-plugin-maru
Summary:    Device manager for emulator
Version: 0.0.16
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: packaging/device-manager-plugin-maru.manifest
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  pkgconfig(devman_plugin)

%description
Device manager for emulator

%prep
%setup -q

%build
export LDFLAGS+="-Wl,--rpath=%{_libdir} -Wl,--as-needed"
%cmake .

make 

%install
%make_install
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/libslp_devman_plugin.so
/usr/share/license/%{name}
