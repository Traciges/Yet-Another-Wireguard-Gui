Name:           yet-another-wireguard-gui
Version:        1.0.2
Release:        1%{?dist}
Summary:        WireGuard VPN Manager for KDE Plasma
License:        GPL-3.0-only
URL:            https://github.com/Traciges/Yet-Another-Wireguard-Gui
Source0:        %{url}/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  ninja-build
BuildRequires:  gcc-c++
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  kf6-kirigami-devel
BuildRequires:  polkit-qt6-1-devel
BuildRequires:  systemd-rpm-macros
BuildRequires:  desktop-file-utils

# Runtime-only deps RPM cannot autodetect: QML imports and wg-quick.
Requires:       qt6-qtdeclarative
Requires:       kf6-kirigami
Requires:       wireguard-tools
Requires:       polkit

%description
Yet Another WireGuard GUI is a KDE Plasma frontend for managing
WireGuard VPN connections. Uses a privileged daemon with D-Bus and
PolicyKit for secure privilege separation.

# ── Prep ─────────────────────────────────────────────────────────────────────

%prep
%autosetup -n Yet-Another-Wireguard-Gui-%{version}

# ── Build ────────────────────────────────────────────────────────────────────

%build
%cmake -GNinja
%cmake_build

# ── Install ──────────────────────────────────────────────────────────────────

%install
%cmake_install

# ── Checks ───────────────────────────────────────────────────────────────────

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/yawg-gui.desktop

# ── Post-install scriptlets ──────────────────────────────────────────────────

%post
%systemd_post yawg-daemon.service
# GUI is unusable until the daemon runs, so don't make the user reboot.
if [ $1 -eq 1 ]; then
    systemctl start yawg-daemon.service || :
fi

%preun
%systemd_preun yawg-daemon.service

%postun
%systemd_postun_with_restart yawg-daemon.service

# ── File manifest ────────────────────────────────────────────────────────────

%files
%license LICENSE
%doc README.md
%{_bindir}/yawg-daemon
%{_bindir}/yawg-gui
%{_datadir}/dbus-1/system.d/io.github.traciges.WireguardManager.conf
%{_datadir}/polkit-1/rules.d/io.github.traciges.wireguard.rules
%{_datadir}/polkit-1/actions/io.github.traciges.wireguard.policy
%{_datadir}/applications/yawg-gui.desktop
%{_datadir}/icons/hicolor/256x256/apps/yawg-gui.png
%{_unitdir}/yawg-daemon.service
%{_presetdir}/50-yawg-daemon.preset

%changelog
* Fri Aug 21 2026 Guido Philipp <guidophilipp2002@gmail.com> - 1.0.2-1
- Build QML with --only-bytecode so the binary no longer depends on
  Qt private API, which pinned it to a single Qt minor release
- Install into %%{_bindir} and %%{_unitdir} instead of /usr/local and
  /etc/systemd/system
