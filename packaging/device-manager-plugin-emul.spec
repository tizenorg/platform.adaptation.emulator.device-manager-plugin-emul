Name:               device-manager-plugin-emul
Summary:            device-manager-plugin-emul
Version:            0.0.16
Release:            1
Group:              SDK/Other
License:            Apache-2.0
Source0:            %{name}-%{version}.tar.gz
Source1001:         packaging/device-manager-plugin-emul.manifest
Requires(post):     /sbin/ldconfig
Requires(postun):   /sbin/ldconfig
BuildRequires:      cmake
BuildRequires:      pkgconfig(dlog)
BuildRequires:      pkgconfig(devman_plugin)

%description
device-manager-plugin-emul.

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
%{_libdir}/libslp_devman_plugin.so

