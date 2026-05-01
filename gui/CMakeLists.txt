find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick DBus QuickControls2 LabsPlatform)
find_package(KF6Kirigami REQUIRED)

set(DBUS_XML ${CMAKE_SOURCE_DIR}/shared/io.github.traciges.WireguardManager.xml)

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
    QML_FILES
        qml/Main.qml
        qml/AddProfileDialog.qml
        qml/DeleteProfileDialog.qml
        qml/ProfileCard.qml
)

qt_add_resources(yawg-gui "appResources"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    FILES
        resources/app-icon.png
        resources/github.svg
)

target_include_directories(yawg-gui PRIVATE
    ${CMAKE_SOURCE_DIR}/shared
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

install(TARGETS yawg-gui DESTINATION /usr/local/bin)
install(FILES yawg-gui.desktop DESTINATION /usr/share/applications/)
install(FILES resources/app-icon.png
    DESTINATION /usr/share/icons/hicolor/256x256/apps/
    RENAME yawg-gui.png)
