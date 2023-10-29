cmake_minimum_required(VERSION 3.14.4)

project(hackers-mc-launcher)

set(AUI_VERSION e6908eca)

set(BUILD_SHARED_LIBS OFF) # there's no reason to use dll build

file(DOWNLOAD https://raw.githubusercontent.com/aui-framework/aui/${AUI_VERSION}/aui.boot.cmake ${CMAKE_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_BINARY_DIR}/aui.boot.cmake)
auib_import(AUI https://github.com/aui-framework/aui
            COMPONENTS core views crypt json curl uitests
            VERSION ${AUI_VERSION})

find_package(ZLIB REQUIRED)
aui_executable(hackers-mc-launcher
        #WIN32_SUBSYSTEM_CONSOLE
        )

aui_compile_assets(hackers-mc-launcher)

target_compile_definitions(hackers-mc-launcher PUBLIC HACKERS_MC_VERSION="1.0.0")

aui_link(hackers-mc-launcher PRIVATE aui::views aui::json aui::crypt aui::curl)
aui_static_link(hackers-mc-launcher minizip)
aui_link(minizip PUBLIC ZLIB::ZLIB)

if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
endif()

aui_enable_tests(hackers-mc-launcher)



aui_app(TARGET hackers-mc-launcher
        NAME "Hacker's MC Launcher"
        ICON icon.svg)
