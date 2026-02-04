cmake_minimum_required(VERSION 3.19)
<<<<<<< HEAD
project(sans_titre LANGUAGES CXX)
=======
project(user LANGUAGES CXX)
>>>>>>> origin/gestion_user

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

<<<<<<< HEAD
qt_add_executable(sans_titre
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(sans_titre
=======
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
>>>>>>> origin/gestion_user
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

<<<<<<< HEAD
install(TARGETS sans_titre
=======
install(TARGETS user
>>>>>>> origin/gestion_user
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
<<<<<<< HEAD
    TARGET sans_titre
=======
    TARGET user
>>>>>>> origin/gestion_user
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
