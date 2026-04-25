find_package(Qt6 REQUIRED COMPONENTS Core DBus Network)
find_package(PolkitQt6-1 REQUIRED)

set(DAEMON_SOURCES
    main.cpp
    WireguardManager.cpp
)

qt_add_dbus_adaptor(DAEMON_SOURCES
    io.github.traciges.WireguardManager.xml
    WireguardManager.h
    WireguardManager
)

add_executable(yawg-daemon ${DAEMON_SOURCES})

target_include_directories(yawg-daemon PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(yawg-daemon
    PRIVATE
    Qt6::Core
    Qt6::DBus
    Qt6::Network
    PolkitQt6-1::Core
)

install(TARGETS yawg-daemon DESTINATION /usr/local/bin)
install(FILES io.github.traciges.WireguardManager.conf
    DESTINATION /etc/dbus-1/system.d/)
install(FILES io.github.traciges.wireguard.rules
    DESTINATION /usr/share/polkit-1/rules.d/)
install(FILES io.github.traciges.wireguard.policy
    DESTINATION /usr/share/polkit-1/actions/)
install(FILES yawg-daemon.service
    DESTINATION /etc/systemd/system/)
