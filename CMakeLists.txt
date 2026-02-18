cmake_minimum_required(VERSION 3.19)

project(PortFlow LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_add_executable(appPortFlow
    main.cpp
    mainwindow.cpp mainwindow.h
    loginwindow.cpp loginwindow.h
    Bateauwindow.cpp Bateauwindow.h
    Bateaudialog.cpp Bateaudialog.h
    Pechewindow.cpp Pechewindow.h
    Pechedialog.cpp Pechedialog.h
    Employeewindow.cpp Employeewindow.h
    Employeedialog.cpp Employeedialog.h
    Frigowindow.cpp Frigowindow.h
    addfrigodialog.cpp addfrigodialog.h
    Livraisonwindow.cpp Livraisonwindow.h
    AddLivraisonDialog.cpp AddLivraisonDialog.h
    dockswindow.cpp dockswindow.h
    adddockdialog.cpp adddockdialog.h
    dock.cpp dock.h
)

target_link_libraries(appPortFlow
    PRIVATE
        Qt::Core
        Qt::Widgets
)

set_target_properties(appPortFlow PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
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
