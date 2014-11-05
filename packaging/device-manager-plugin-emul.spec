Name:               device-manager-plugin-emul
Summary:            Device manager plugin for emulator
Version:            0.0.16
Release:            1
Group:              SDK/Other
License:            Apache-2.0
Source0:            %{name}-%{version}.tar.gz
Requires(post):     /sbin/ldconfig
Requires(postun):   /sbin/ldconfig
BuildRequires:      cmake
BuildRequires:      pkgconfig(dlog)
BuildRequires:      pkgconfig(devman_plugin)

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
%defattr(-,root,root,-)
%manifest device-manager-plugin-emul.manifest
%{_libdir}/libslp_devman_plugin.so

