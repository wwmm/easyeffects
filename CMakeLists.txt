cmake_minimum_required(VERSION 3.28)

project(easyeffects VERSION 8.0.0 LANGUAGES CXX C)

set(APPLICATION_DOMAIN "easyeffects")
set(APPLICATION_NAME "EasyEffects")
set(COMPONENT_NAME "easyeffects")
set(ORGANIZATION_NAME "WWMM")
set(ORGANIZATION_DOMAIN "com.github.wwmm")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(QT_DEFAULT_MAJOR_VERSION 6)
set(QT_MAJOR_VERSION ${QT_DEFAULT_MAJOR_VERSION})

set(Boost_USE_MULTITHREADED ON)

find_package(ECM REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt${QT_MAJOR_VERSION} REQUIRED NO_MODULE COMPONENTS
    Core
    Quick
    Qml
    Gui
    QuickControls2
    Widgets
    Charts
)

find_package(KF${QT_MAJOR_VERSION} REQUIRED COMPONENTS
    Kirigami
    I18n
    CoreAddons
    Config
    ConfigWidgets
)

ki18n_install(po)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBPIPEWIRE libpipewire-0.3>=1.2.1)

file(GLOB KCFG_FILES ${PROJECT_SOURCE_DIR}/src/contents/kcfg/*.kcfg)
file(GLOB KCFGC_FILES ${PROJECT_SOURCE_DIR}/src/contents/kcfg/*.kcfgc)

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
