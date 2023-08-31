cmake_minimum_required (VERSION 3.6)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Installation directory" FORCE)
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

set(LIB_NAME "gotcp" CACHE STRING "")
project (${LIB_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SELF_LIBS_OUT ${CMAKE_SYSTEM_NAME}.out)

# default result is library
set(BUILD_RET "LIB" CACHE STRING "")
message("build result: ${BUILD_RET}")

# default with ssl
set(SSL "1" CACHE STRING "")
message("SSL: ${SSL}")

if (${SSL} EQUAL "1")
    include_directories(openssl/include)
    # so we could use WITH_SSL as preprocessor command in codes
    ADD_DEFINITIONS(-DWITH_SSL)
endif()
include_directories(.)
include_directories(..)
include_directories(common)
include_directories(common/android)
include_directories(socket)
include_directories(packet)
include_directories(connect)
include_directories(log)

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} *.cpp *.c *.hpp)
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} common/*.cpp common/*.c common/*.hpp)
source_group(common FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} common/android/*.cpp common/android/*.c common/android/*.hpp)
source_group(common FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} log/*.cpp log/*.c log/*.hpp)
source_group(common FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} socket/*.cpp socket/*.hpp)
source_group(socket FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} packet/*.cpp packet/*.hpp)
source_group(packet FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} connect/*.cpp connect/*.hpp)
source_group(connect FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

if (${SSL} EQUAL "0")
    list(REMOVE_ITEM SELF_SRC_FILES socket/ssl_socket.cpp)
    list(REMOVE_ITEM SELF_SRC_FILES connect/ssl_connector.cpp)
endif()

#for log on different platform
if (ANDROID)
    file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} log/android/*.cpp)
    source_group(common FILES ${SELF_TEMP_SRC_FILES})
    list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})
elseif (APPLE)
    file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} log/objc/*.mm)
    source_group(common FILES ${SELF_TEMP_SRC_FILES})
    list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})
elseif (LINUX)
    file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} log/linux/*.cpp)
    source_group(common FILES ${SELF_TEMP_SRC_FILES})
    list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})
endif()

if (${BUILD_RET} STREQUAL "LIB")
    add_library(${PROJECT_NAME} STATIC ${SELF_SRC_FILES})
    install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${CMAKE_SYSTEM_NAME}.out)
else()
    add_executable(${PROJECT_NAME} main.cpp ${SELF_SRC_FILES})

    find_library(z-lib z)
    target_link_libraries(${PROJECT_NAME}
                        ${z-lib})
    if (APPLE)
        find_library(FOUNDATION Foundation)
        target_link_libraries(${PROJECT_NAME}
                            ${FOUNDATION})
        if (${SSL} EQUAL "1")
            message("LINK OSX OPENSSL")
            target_link_libraries(${PROJECT_NAME}
                            ${PROJECT_SOURCE_DIR}/openssl/osx/libssl.a
                            ${PROJECT_SOURCE_DIR}/openssl/osx/libcrypto.a)
        endif()
    else()
        target_link_libraries(${PROJECT_NAME}
                            pthread
                            dl)
        if (${SSL} EQUAL "1")
            message("LINK linux OPENSSL")
            target_link_libraries(${PROJECT_NAME}
                            ${PROJECT_SOURCE_DIR}/openssl/linux/libssl.a
                            ${PROJECT_SOURCE_DIR}/openssl/linux/libcrypto.a)
        endif()
    endif()
endif()

if (ANDROID)
    find_library(z-lib z)
    find_library(log-lib log)

    link_directories(.)

    set(SELF_LIB_NAME ${LIB_NAME}_shared)

    file(GLOB SELF_ANDROID_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR}
            jni/*.cc)

    list(APPEND SELF_SRC_FILES ${SELF_ANDROID_SRC_FILES})

    add_library(${SELF_LIB_NAME} SHARED ${SELF_SRC_FILES})

    install(TARGETS ${SELF_LIB_NAME} LIBRARY DESTINATION ${SELF_LIBS_OUT} ARCHIVE DESTINATION ${SELF_LIBS_OUT})

    target_link_libraries(${SELF_LIB_NAME} "-Wl,--gc-sections"
                           ${z-lib}
                           ${log-lib})

    if (${SSL} EQUAL "1")
        message("LINK ANDROID OPENSSL")
        set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")

        find_library(CRYPT_LIB crypto PATHS openssl/android/${ANDROID_ABI} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
        find_library(SSL_LIB ssl PATHS openssl/android/${ANDROID_ABI} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

        target_link_libraries(${SELF_LIB_NAME} "-Wl,--gc-sections"
                        ${SSL_LIB}
                        ${CRYPT_LIB})
    endif()
endif()
