cmake_minimum_required(VERSION 3.16)
project(hackers-mc-launcher
        VERSION 0.9.0 # CI_PROJECT_VERSION
)

set(AUI_VERSION v8.0.0-rc.16)

option(BUILD_SHARED_LIBS "Build shared libs" OFF) # there's no reason to use dll build

file(DOWNLOAD https://raw.githubusercontent.com/aui-framework/aui/${AUI_VERSION}/aui.boot.cmake ${CMAKE_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_BINARY_DIR}/aui.boot.cmake)
auib_import(AUI https://github.com/aui-framework/aui
            COMPONENTS core views crypt json curl uitests updater
            VERSION ${AUI_VERSION})

find_package(ZLIB REQUIRED)
aui_executable(${CMAKE_PROJECT_NAME}
        #WIN32_SUBSYSTEM_CONSOLE
        )

aui_compile_assets(${CMAKE_PROJECT_NAME})

aui_link(${CMAKE_PROJECT_NAME} PRIVATE aui::views aui::json aui::crypt aui::curl aui::updater)

if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
endif()

aui_enable_tests(${CMAKE_PROJECT_NAME})



aui_app(TARGET ${CMAKE_PROJECT_NAME}
        NAME "Hacker's MC Launcher"
        ICON icon.svg)
