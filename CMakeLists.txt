cmake_minimum_required(VERSION 3.16)
project(ChangeIPTool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    IpConfigManager.cpp
    IpConfigManager.h
    NetworkAdapterManager.cpp
    NetworkAdapterManager.h
)

qt_add_executable(ChangeIPTool
    ${PROJECT_SOURCES}
)

qt_add_resources(ChangeIPTool "resources"
    PREFIX "/"
    FILES
        icons/apply.ico
)

target_link_libraries(ChangeIPTool PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# Windows specific settings
if(WIN32)
    set_target_properties(ChangeIPTool PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    # Enable console for debugging
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(ChangeIPTool PRIVATE console子系统)
    endif()
endif()

# Installation
install(TARGETS ChangeIPTool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# CPack configuration for Windows
set(CPACK_PACKAGE_NAME "ChangeIPTool")
set(CPACK_PACKAGE_VENDOR "IPTool")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IP Address Changer Tool")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ChangeIPTool")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "ChangeIPTool")
    set(CPACK_NSIS_PACKAGE_NAME "ChangeIPTool")
    set(CPACK_NSIS_CONTACT "support@example.com")
    set(CPACK_NSIS_HELP_LINK "https://github.com/yourusername/ChangeIPTool")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/yourusername/ChangeIPTool")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "ChangeIPTool.exe")
    set(CPACK_NSIS_MENU_LINKS "bin/ChangeIPTool.exe" "ChangeIPTool")

    # Create a desktop shortcut
    set(CPACK_NSIS_CREATE_ICONS_EXTRA "
        CreateShortCut '$DESKTOP\\ChangeIPTool.lnk' '$INSTDIR\\bin\\ChangeIPTool.exe'
    ")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA "
        Delete '$DESKTOP\\ChangeIPTool.lnk'
    ")
endif()

include(CPack)
