cmake_minimum_required(VERSION 3.14.4)

project(hackers-mc-launcher)

file(DOWNLOAD https://raw.githubusercontent.com/aui-framework/aui/master/aui.boot.cmake aui.boot.cmake)
include(${CMAKE_BINARY_DIR}/aui.boot.cmake)
auib_import(AUI https://github.com/aui-framework/aui
            COMPONENTS core views crypt json curl)

find_package(ZLIB REQUIRED)

aui_executable_advanced(hackers-mc-launcher "${ADDITIONAL_SRCS}")

aui_compile_assets(hackers-mc-launcher)

target_compile_definitions(hackers-mc-launcher PUBLIC HACKERS_MC_VERSION="1.0.0")

target_link_libraries(hackers-mc-launcher PRIVATE aui::views aui::json aui::crypt aui::curl)
aui_static_link(hackers-mc-launcher minizip)
target_link_libraries(minizip PUBLIC ZLIB::ZLIB)

if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
endif()
