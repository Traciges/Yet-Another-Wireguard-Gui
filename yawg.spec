Name:           yet-another-wireguard-gui
Version:        0.1.0
Release:        1%{?dist}
Summary:        WireGuard VPN Manager for KDE Plasma
License:        GPL-3.0-only
URL:            https://github.com/Traciges/Yet-Another-Wireguard-Gui

BuildRequires:  cmake
BuildRequires:  ninja-build
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qtbase-gui
BuildRequires:  kf6-kirigami-devel
BuildRequires:  polkit-qt6-1-devel

Requires:       qt6-qtbase
Requires:       qt6-qtdeclarative
Requires:       kf6-kirigami
Requires:       polkit
Requires:       wireguard-tools
Requires(post): systemd

%description
Yet Another WireGuard GUI is a KDE Plasma frontend for managing
WireGuard VPN connections. Uses a privileged daemon with D-Bus and
PolicyKit for secure privilege separation.

# ── Prep ─────────────────────────────────────────────────────────────────────

%prep
cp %{_sourcedir}/LICENSE .

# ── Build ────────────────────────────────────────────────────────────────────

%build
cmake -S %{_sourcedir} -B %{_builddir}/build \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja
ninja -C %{_builddir}/build

# ── Install ──────────────────────────────────────────────────────────────────

%install
DESTDIR=%{buildroot} ninja -C %{_builddir}/build install
install -Dm644 %{_sourcedir}/packaging/50-yawg-daemon.preset \
    %{buildroot}%{_presetdir}/50-yawg-daemon.preset

# ── Post-install scriptlets ──────────────────────────────────────────────────

%post
systemctl daemon-reload
%systemd_post yawg-daemon.service
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
/usr/local/bin/yawg-daemon
/usr/local/bin/yawg-gui
%config /etc/dbus-1/system.d/io.github.traciges.WireguardManager.conf
/usr/share/polkit-1/rules.d/io.github.traciges.wireguard.rules
/usr/share/polkit-1/actions/io.github.traciges.wireguard.policy
/etc/systemd/system/yawg-daemon.service
%{_presetdir}/50-yawg-daemon.preset
/usr/share/applications/yawg-gui.desktop
/usr/share/icons/hicolor/256x256/apps/yawg-gui.png
