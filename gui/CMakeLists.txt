find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick DBus QuickControls2 LabsPlatform)
find_package(KF6Kirigami REQUIRED)

set(DBUS_XML ${CMAKE_SOURCE_DIR}/daemon/org.example.WireguardManager.xml)

set_source_files_properties(${DBUS_XML}
    PROPERTIES INCLUDE "WireguardTypes.h"
)

set(GUI_SOURCES
    main.cpp
    WireguardManagerBridge.cpp
)

qt_add_dbus_interface(GUI_SOURCES ${DBUS_XML} WireguardManagerProxy)

add_executable(yawg-gui ${GUI_SOURCES})

qt_add_qml_module(yawg-gui
    URI YetAnotherWireguardGui
    VERSION 1.0
    QML_FILES Main.qml
)

qt_add_resources(yawg-gui "appResources"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    FILES
        resources/app-icon.png
)

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
    Qt6::Widgets
    Qt6::QuickControls2
    Qt6::LabsPlatform
    KF6::Kirigami
)
