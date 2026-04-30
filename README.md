# Yet Another Wireguard Gui

KDE Plasma frontend for WireGuard (sentinel architecture: root daemon + Kirigami GUI via D-Bus).

## Install Dependencies

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
# If libkf6kirigami-dev is missing:
# sudo add-apt-repository ppa:kubuntu-ppa/backports
```

## Build

```bash
mkdir build && cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug
ninja
```

## One-Time System Configuration (requires root)

```bash
# Install D-Bus policy
sudo install -m 644 daemon/org.example.WireguardManager.conf /etc/dbus-1/system.d/
sudo systemctl reload dbus

# Place WireGuard test profile
sudo install -m 600 /path/to/YourProfile.conf /etc/wireguard/
```

## Run

```bash
# Terminal 1 - Daemon (root)
sudo ./build/daemon/yawg-daemon

# Terminal 2 - GUI
./build/gui/yawg-gui
```

## Test D-Bus Interface

```bash
busctl call org.example.WireguardManager \
    /org/example/WireguardManager \
    org.example.WireguardManager \
    ListProfiles
```
