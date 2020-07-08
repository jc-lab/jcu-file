cmake_minimum_required(VERSION 3.12)
project(jcu-file CXX)

# <CONAN BUILD>

set(SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/jcu-file/path.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/jcu-file/file-handler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/jcu-file/file-factory.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/path.cc
        )

if (WIN32)
    set(SRC_PLATFORM_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/include/jcu-file/win32/win-file-handler.h
            ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/win-file-handler.cc
            )
endif ()

add_library(${PROJECT_NAME} ${SRC_FILES} ${SRC_PLATFORM_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(${PROJECT_NAME} jcu-random)
