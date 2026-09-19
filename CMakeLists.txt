cmake_minimum_required(VERSION 3.21)

project(SimpleMapMarkerControls
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_TESTS OFF CACHE BOOL "Build CommonLibSSE-NG tests" FORCE)

add_subdirectory(external/CommonLibSSE-NG)

include(external/CommonLibSSE-NG/cmake/CommonLibSSE.cmake)

add_commonlibsse_plugin(${PROJECT_NAME}
        SOURCES
        src/main.cpp
)

target_precompile_headers(${PROJECT_NAME}
        PRIVATE
        src/PCH.hpp
)