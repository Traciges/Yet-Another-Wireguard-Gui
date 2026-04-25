find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick DBus)
find_package(KF6Kirigami REQUIRED)

set(DBUS_XML ${CMAKE_SOURCE_DIR}/daemon/org.example.WireguardManager.xml)

# Tell qdbusxml2cpp to include our custom type header in the generated proxy
set_source_files_properties(${DBUS_XML}
    PROPERTIES INCLUDE "WireguardTypes.h"
)

set(GUI_SOURCES main.cpp)

qt_add_dbus_interface(GUI_SOURCES ${DBUS_XML} WireguardManagerProxy)

add_executable(yawg-gui ${GUI_SOURCES})

qt_add_qml_module(yawg-gui
    URI YetAnotherWireguardGui
    VERSION 1.0
    QML_FILES Main.qml
)

# Expose daemon headers (WireguardTypes.h) and the generated proxy header
target_include_directories(yawg-gui PRIVATE
    ${CMAKE_SOURCE_DIR}/daemon
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(yawg-gui
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::DBus
    KF6::Kirigami
)
