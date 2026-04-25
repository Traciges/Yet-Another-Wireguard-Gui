# Yet Another Wireguard GUI

KDE Plasma Frontend für WireGuard (Sentinel-Architektur: Root-Daemon + Kirigami-GUI via D-Bus).

## Abhängigkeiten installieren

### Fedora
```bash
sudo dnf install \
    cmake ninja-build extra-cmake-modules \
    qt6-qtbase-devel qt6-qtdeclarative-devel \
    kf6-kirigami-devel \
    polkit-devel
```

### Arch Linux
```bash
sudo pacman -S --needed \
    cmake ninja extra-cmake-modules \
    qt6-base qt6-declarative qt6-tools \
    kf6-kirigami polkit-qt6
```

### Ubuntu 24.04
```bash
sudo apt install \
    cmake ninja-build extra-cmake-modules \
    qt6-base-dev qt6-declarative-dev \
    libkf6kirigami-dev \
    libpolkit-qt6-1-dev
# Falls libkf6kirigami-dev fehlt:
# sudo add-apt-repository ppa:kubuntu-ppa/backports
```

## Bauen

```bash
mkdir build && cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug
ninja
```

## Einmalige Systemkonfiguration (braucht root)

```bash
# D-Bus Policy installieren
sudo install -m 644 daemon/org.example.WireguardManager.conf /etc/dbus-1/system.d/
sudo systemctl reload dbus

# WireGuard-Testprofil ablegen
sudo install -m 600 /path/to/YourProfile.conf /etc/wireguard/
```

## Starten

```bash
# Terminal 1 — Daemon (root)
sudo ./build/daemon/yawg-daemon

# Terminal 2 — GUI
./build/gui/yawg-gui
```

## D-Bus Interface testen

```bash
busctl call org.example.WireguardManager \
    /org/example/WireguardManager \
    org.example.WireguardManager \
    ListProfiles
```
