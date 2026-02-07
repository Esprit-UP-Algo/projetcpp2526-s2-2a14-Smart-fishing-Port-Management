cmake_minimum_required(VERSION 3.19)
project(gestion_bateaux LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(gestion_bateaux
    WIN32 MACOSX_BUNDLE
    main.cpp
    Bateaudialog.cpp
    Bateauwindow.h
    bateau.ui
    Bateaudialog.h
    Bateauwindow.cpp
)

target_link_libraries(gestion_bateaux
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS gestion_bateaux
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET gestion_bateaux
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
