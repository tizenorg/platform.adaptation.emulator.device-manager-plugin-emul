Name:       device-manager-plugin-maru
Summary:    device-manager-plugin-maru
Version: 0.0.15
Release:    1
Group:      TO_BE/FILLED_IN
License:    TO_BE/FILLED_IN
Source0:    %{name}-%{version}.tar.gz
Source1001: packaging/device-manager-plugin-maru.manifest
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  pkgconfig(devman_plugin)

%description
device-manager-plugin-maru.

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

