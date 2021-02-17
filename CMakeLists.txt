cmake_minimum_required(VERSION 3.14.4)

project(hackers-ms-launcher)

set(AUI_USE_OLD_WAY false CACHE BOOL "The old linking way to AUI")

if(AUI_USE_OLD_WAY)
    set(AUI_DIR "AUI_DIR" CACHE PATH "AUI DIR PATH")
    add_subdirectory(${AUI_DIR} build)
else()
    message("Using new way to link AUI")
    find_package(AUI REQUIRED)
endif()

find_package(ZLIB REQUIRED)

AUI_Deploy_Library(AUI.Svg)
AUI_Deploy_Library(AUI.Image)

AUI_Executable_Advanced(hackers-ms-launcher "${ADDITIONAL_SRCS}")

AUI_Compile_Assets(hackers-ms-launcher)

target_link_libraries(hackers-ms-launcher PRIVATE AUI.Core)
target_link_libraries(hackers-ms-launcher PRIVATE AUI.Data)
target_link_libraries(hackers-ms-launcher PRIVATE AUI.Views)
target_link_libraries(hackers-ms-launcher PRIVATE AUI.Crypt)
target_link_libraries(hackers-ms-launcher PRIVATE AUI.Curl)
target_link_libraries(hackers-ms-launcher PRIVATE AUI.Json)
#AUI_Static_Link(hackers-ms-launcher minizip)
#target_link_libraries(minizip PUBLIC ZLIB::ZLIB)

if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
endif()
