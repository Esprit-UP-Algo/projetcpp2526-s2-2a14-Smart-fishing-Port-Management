cmake_minimum_required(VERSION 3.19)

project(PortFlow LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(appPortFlow
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    loginwindow.cpp loginwindow.h loginwindow.ui
    Bateauwindow.cpp Bateauwindow.h
    Bateaudialog.cpp Bateaudialog.h
    Pechewindow.cpp Pechewindow.h Pechewindow.ui
    Pechedialog.cpp Pechedialog.h
    Employeewindow.cpp Employeewindow.h
    Employeedialog.cpp Employeedialog.h
    Frigowindow.cpp Frigowindow.h
    addfrigodialog.cpp addfrigodialog.h
)

target_link_libraries(appPortFlow
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS appPortFlow
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET appPortFlow
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
