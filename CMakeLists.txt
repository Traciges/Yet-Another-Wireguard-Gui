cmake_minimum_required(VERSION 3.20)
project(YetAnotherWireguardGui VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

# Arch-independent, so <prefix>/lib -- not CMAKE_INSTALL_LIBDIR (lib64 here).
set(SYSTEMD_UNIT_DIR "${CMAKE_INSTALL_PREFIX}/lib/systemd/system"
    CACHE PATH "systemd system unit directory")

find_package(Qt6 REQUIRED COMPONENTS Core DBus)

add_subdirectory(daemon)
add_subdirectory(gui)
