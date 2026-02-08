cmake_minimum_required(VERSION 3.19)

project(gestion_bateaux LANGUAGES CXX)

project(gestion_peche LANGUAGES CXX)


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

qt_add_executable(gestion_peche
    WIN32 MACOSX_BUNDLE
    main.cpp
    Pechewindow.cpp
    Pechewindow.h
    Pechewindow.ui
    Pechedialog.h
    Pechedialog.cpp
    Employeedialog.cpp
    Employeedialog.h
    Employeewindow.cpp
    Employeewindow.h
    Frigowindow.cpp
    Frigowindow.h
    addfrigodialog.cpp
    addfrigodialog.h
    loginwindow.cpp
    loginwindow.h
    loginwindow.ui
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(gestion_peche

    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)


install(TARGETS gestion_bateaux

install(TARGETS gestion_peche

    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(

    TARGET gestion_bateaux

    TARGET gestion_peche

    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
