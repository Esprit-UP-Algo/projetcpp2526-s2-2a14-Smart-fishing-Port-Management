cmake_minimum_required(VERSION 3.19)
project(user LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(user
    WIN32 MACOSX_BUNDLE
    main.cpp
    loginwindow.cpp
    loginwindow.h
    loginwindow.ui
    Employeewindow.h
    Employeedialog.h
    Employeewindow.cpp
    Employeedialog.cpp
)

target_link_libraries(user
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS user
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET user
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
