cmake_minimum_required(VERSION 3.12)
project(jcu-file CXX)

# <CONAN BUILD>

include(FetchContent)

if (NOT TARGET jcu-random)
    FetchContent_Declare(
            jcu_random
            GIT_REPOSITORY https://github.com/jc-lab/jcu-random.git
            GIT_TAG master
    )

    FetchContent_GetProperties(jcu_random)

    if (NOT jcu_random_POPULATED)
        FetchContent_Populate(jcu_random)
        add_subdirectory(${jcu_random_SOURCE_DIR} ${jcu_random_BINARY_DIR})
    endif ()
endif ()

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
