cmake_minimum_required(VERSION 3.28)

project(easyeffects VERSION 8.0.6 LANGUAGES CXX C)

set(APPLICATION_DOMAIN "easyeffects")
set(COMPONENT_NAME "easyeffects")
set(ORGANIZATION_NAME "WWMM")
set(ORGANIZATION_DOMAIN "com.github.wwmm")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(QT_MAJOR_VERSION 6)

set(QT_QML_GENERATE_QMLLS_INI ON)

option(ENABLE_DEVEL "Whether to use development build mode, with .Devel appended to the application ID. For Flatpak builds, user data for development builds is stored separately from stable builds." OFF)
option(ENABLE_RNNOISE "Whether to use RNNoise for noise cancellation" ON)
option(ENABLE_LIBCPP_WORKAROUNDS "Enabled Workarounds for systems that use libc++ instead of stdc++" OFF)
option(ENABLE_SANITIZER "Enable the compiler's sanitizer" OFF)
option(BUILD_FLATPAK "Configure easyeffects to target flatpak. If disabled, dependency to libportal is omitted" ON)

if(ENABLE_DEVEL)
    message(STATUS "Using development build mode with .Devel appended to the application ID.")
    set(APPLICATION_ID "com.github.wwmm.easyeffects.Devel")
    set(APPLICATION_NAME "Easy Effects (Devel)")
else()
    message(STATUS "Using stable build mode with the standard application ID")
    set(APPLICATION_ID "com.github.wwmm.easyeffects")
    set(APPLICATION_NAME "Easy Effects")
endif()

find_package(ECM REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMFindQmlModule)
include(ECMQmlModule)

find_package(Qt${QT_MAJOR_VERSION} REQUIRED NO_MODULE COMPONENTS
    Core
    DBus
    Graphs
    Gui
    Network
    Qml
    Quick
    QuickControls2
    Widgets
    WebEngineQuick
)

find_package(KF6 REQUIRED COMPONENTS
    ColorScheme
    Config
    ConfigWidgets
    CoreAddons
    I18n
    IconThemes
    Kirigami
    KirigamiAddons
    QQC2DesktopStyle
)

ecm_find_qmlmodule(org.kde.kirigami REQUIRED)

qt_policy(SET QTP0001 NEW)

ki18n_install(po)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(TBB REQUIRED)
find_package(GSL REQUIRED)
# find_package(SoundTouch REQUIRED) # As of SoundTouch 2.4.0 its cmake files are bugged

pkg_check_modules(LIBPIPEWIRE libpipewire-0.3>=1.0.6 IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBLILV lilv-0>=0.24 IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBEBUR128 libebur128>=1.2.6 IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBFFTW3 fftw3 IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBFFTW3f fftw3f IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBSPEEXDSP speexdsp IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBNLOHMANNJSON nlohmann_json IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBGSL gsl IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBBS2B libbs2b IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBSAMPLERATE samplerate IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBSNDFILE sndfile IMPORTED_TARGET REQUIRED)
if(BUILD_FLATPAK OR ENABLE_RNNOISE)
    pkg_check_modules(LIBRNNOISE rnnoise IMPORTED_TARGET REQUIRED)
endif()
pkg_check_modules(LIBSOUNDTOUCH soundtouch IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBPORTAL libportal IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBPORTALQT libportal-qt6 IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBWEBRTC webrtc-audio-processing-2 IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBMYSOFA libmysofa IMPORTED_TARGET REQUIRED)

file(GLOB KCFG_FILES ${PROJECT_SOURCE_DIR}/src/contents/kcfg/*.kcfg)
file(GLOB KCFGC_FILES ${PROJECT_SOURCE_DIR}/src/contents/kcfg/*.kcfgc)

# Remove RNNoise kcfg if disabled
if(NOT BUILD_FLATPAK AND NOT ENABLE_RNNOISE)
    list(REMOVE_ITEM KCFG_FILES ${PROJECT_SOURCE_DIR}/src/contents/kcfg/easyeffects_db_rnnoise.kcfg)
    list(REMOVE_ITEM KCFGC_FILES ${PROJECT_SOURCE_DIR}/src/contents/kcfg/easyeffects_db_rnnoise.kcfgc)
endif()

find_library(LIBZITACONVOLVER NAMES zita-convolver)
find_path(LIBZITACONVOLVER_INCLUDE_DIRS NAMES zita-convolver.h)

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
