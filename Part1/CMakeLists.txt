cmake_minimum_required(VERSION 3.9)

project(ACG LANGUAGES C CXX)

# Directories
set(DIR_ROOT       ${CMAKE_CURRENT_LIST_DIR})
set(DIR_SOURCES    "${DIR_ROOT}/src")


# Init with Debug mode
if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
    endif()
endif()


# Macro to map filters to folder structure for MSVC projects
macro(GroupSources curdir)
    if(MSVC)
		file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

        foreach(child ${children})
            if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
                GroupSources(${curdir}/${child})
            else()
                string(REPLACE "/" "\\" groupname ${curdir})
                source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
            endif()
        endforeach()
    endif()
endmacro()

GroupSources(src)

# Sources
macro(ACG_SOURCES_APPEND)
    file(GLOB FILES_APPEND CONFIGURE_DEPENDS ${ARGV0}/*.h)
    list(APPEND ACG_HEADERS ${FILES_APPEND})
    file(GLOB FILES_APPEND CONFIGURE_DEPENDS ${ARGV0}/*.cpp)
    list(APPEND ACG_SOURCES ${FILES_APPEND})
endmacro()

ACG_SOURCES_APPEND(${DIR_SOURCES})
ACG_SOURCES_APPEND(${DIR_SOURCES}/cameras)
ACG_SOURCES_APPEND(${DIR_SOURCES}/core)
ACG_SOURCES_APPEND(${DIR_SOURCES}/lightsources)
ACG_SOURCES_APPEND(${DIR_SOURCES}/materials)
ACG_SOURCES_APPEND(${DIR_SOURCES}/shaders)
ACG_SOURCES_APPEND(${DIR_SOURCES}/shapes)

add_executable(${PROJECT_NAME} ${ACG_SOURCES} ${ACG_HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${DIR_SOURCES})

set_property(DIRECTORY ${DIR_ROOT} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${DIR_ROOT}")

# Properties
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)

# Ensure that _AMD64_ or _X86_ are defined on Microsoft Windows, as otherwise
# um/winnt.h provided since Windows 10.0.22000 will error.
if(NOT UNIX)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions(-D_AMD64_)
        message(STATUS "64 bits detected")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        add_definitions(-D_X86_)
        message(STATUS "32 bits detected")
    endif()
endif(NOT UNIX)


message(STATUS "dir root: ${DIR_ROOT}")
message(STATUS "bin root: ${CMAKE_BINARY_DIR}")