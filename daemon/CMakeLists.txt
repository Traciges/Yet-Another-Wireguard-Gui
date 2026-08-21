find_package(Qt6 REQUIRED COMPONENTS Core DBus Network)
find_package(PolkitQt6-1 REQUIRED)

set(DAEMON_SOURCES
    main.cpp
    WireguardManager.cpp
)

qt_add_dbus_adaptor(DAEMON_SOURCES
    ${CMAKE_SOURCE_DIR}/shared/io.github.traciges.WireguardManager.xml
    WireguardManager.h
    WireguardManager
)

add_executable(yawg-daemon ${DAEMON_SOURCES})

target_include_directories(yawg-daemon PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/shared
)

target_link_libraries(yawg-daemon
    PRIVATE
    Qt6::Core
    Qt6::DBus
    Qt6::Network
    PolkitQt6-1::Core
)

configure_file(yawg-daemon.service.in yawg-daemon.service @ONLY)

install(TARGETS yawg-daemon DESTINATION ${CMAKE_INSTALL_BINDIR})

# <prefix>/share, not /etc -- /etc is for admin overrides of these files.
install(FILES io.github.traciges.WireguardManager.conf
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/system.d)
install(FILES io.github.traciges.wireguard.rules
    DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/rules.d)
install(FILES io.github.traciges.wireguard.policy
    DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/actions)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/yawg-daemon.service
    DESTINATION ${SYSTEMD_UNIT_DIR})
install(FILES ${CMAKE_SOURCE_DIR}/packaging/50-yawg-daemon.preset
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system-preset)
