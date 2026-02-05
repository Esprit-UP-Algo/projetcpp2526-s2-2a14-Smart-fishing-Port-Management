cmake_minimum_required(VERSION 3.19)

project(integration_project LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

# =============================
# Exécutable : sans_titre
# =============================
qt_add_executable(sans_titre
    WIN32 MACOSX_BUNDLE
    main.cpp
    loginwindow.cpp
    loginwindow.h
    loginwindow.ui
    dashboardwindow.cpp
    dashboardwindow.h
    addfrigodialog.cpp
    addfrigodialog.h
    Employeewindow.cpp
    Employeewindow.h
    Employeedialog.cpp
    Employeedialog.h
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(sans_titre
    PRIVATE Qt::Core Qt::Widgets
)

# =============================
# Install
# =============================
include(GNUInstallDirs)

install(TARGETS sans_titre
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# =============================
# Déploiement
# =============================
qt_generate_deploy_app_script(
    TARGET sans_titre
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
