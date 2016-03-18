Name:               device-manager-plugin-emul
Summary:            Device manager plugin for emulator
Version:            0.0.17
Release:            1
Group:              SDK/Other
License:            Apache-2.0
Source0:            %{name}-%{version}.tar.gz
Requires(post):     /sbin/ldconfig
Requires(postun):   /sbin/ldconfig
BuildRequires:      cmake
BuildRequires:      pkgconfig(dlog)
BuildRequires:      pkgconfig(devman_plugin)
BuildRequires:      pkgconfig(hwcommon)

%description
Emulator plugin for libdevice-node.

%prep
%setup -q

%build
export LDFLAGS+="-Wl,--rpath=%{_libdir} -Wl,--as-needed"
%cmake .

make

%install
%make_install

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%license LICENSE
%defattr(-,root,root,-)
%manifest device-manager-plugin-emul.manifest
%{_libdir}/libslp_devman_plugin.so
%{_libdir}/hw/*.so

