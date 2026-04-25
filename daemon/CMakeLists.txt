find_package(Qt6 REQUIRED COMPONENTS Core DBus)
find_package(PolkitQt6-1 QUIET)   # Stub — wird in Meilenstein 2 aktiviert

set(DAEMON_SOURCES
    main.cpp
    WireguardManager.cpp
)

qt_add_dbus_adaptor(DAEMON_SOURCES
    org.example.WireguardManager.xml
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
)

install(TARGETS yawg-daemon DESTINATION /usr/local/bin)
install(FILES org.example.WireguardManager.conf
    DESTINATION /etc/dbus-1/system.d/)
install(FILES com.example.wireguard.rules
    DESTINATION /usr/share/polkit-1/rules.d/)
