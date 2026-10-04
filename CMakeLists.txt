cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

project(HingeCpp VERSION 0.1 LANGUAGES CXX)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3
    GIT_SHALLOW TRUE
)
set(HTTPLIB_REQUIRE_OPENSSL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(httplib)

file(GLOB_RECURSE SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

set(lines "Files:")
foreach(f ${SOURCES})
    string(APPEND lines "\n  ${f}")
endforeach()
message(STATUS "${lines}")
add_executable(${PROJECT_NAME} ${SOURCES})

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
        -include
        "${CMAKE_SOURCE_DIR}/src/compat/windows_prelude.h"
    )
endif()

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    WEB_DIR="${CMAKE_SOURCE_DIR}/web"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    nlohmann_json::nlohmann_json
    httplib::httplib
)

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE ssl crypto)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
