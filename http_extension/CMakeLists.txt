cmake_minimum_required(VERSION 3.16)
project(http_mongodb_ext)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings for 32-bit
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
    add_definitions(-DWIN32 -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -DSOURCEMOD_BUILD)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -m32")
    add_definitions(-DSOURCEMOD_BUILD -DPOSIX -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp)
endif()

# SourceMod SDK paths
set(SOURCEMOD_PATH $ENV{SOURCEMOD_PATH} CACHE PATH "Path to SourceMod SDK")
set(HL2SDK_PATH $ENV{HL2SDK_PATH} CACHE PATH "Path to HL2 SDK")

if(NOT SOURCEMOD_PATH)
    message(FATAL_ERROR "SOURCEMOD_PATH must be set")
endif()

if(NOT HL2SDK_PATH)
    message(FATAL_ERROR "HL2SDK_PATH must be set")
endif()

# Find libcurl (for HTTP requests) - make optional for minimal build
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL libcurl)

# Include directories
include_directories(
    /root/sourcemod-workspace/sourcemod/public
    /root/sourcemod-workspace/sourcemod/public/extensions
    /root/sourcemod-workspace/sourcemod/public/sourcepawn
    /root/sourcemod-workspace/sourcemod/public/amtl
    /root/sourcemod-workspace/sourcemod/public/amtl/amtl
    /root/sourcemod-workspace/sourcemod/sourcepawn/include
    /root/sourcemod-workspace/hl2sdk-tf2/public
    /root/sourcemod-workspace/hl2sdk-tf2/public/engine
    /root/sourcemod-workspace/hl2sdk-tf2/public/mathlib
    /root/sourcemod-workspace/hl2sdk-tf2/public/vstdlib
    /root/sourcemod-workspace/hl2sdk-tf2/public/tier0
    /root/sourcemod-workspace/hl2sdk-tf2/public/tier1
    /root/sourcemod-workspace/hl2sdk-tf2/game/shared
    /root/sourcemod-workspace/hl2sdk-tf2/game/server
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CURL_INCLUDE_DIRS}
    /usr/include/i386-linux-gnu
    /usr/include/x86_64-linux-gnu
)

# Source files
set(SOURCES
    complete_extension.cpp
    /root/sourcemod-workspace/sourcemod/public/smsdk_ext.cpp
)

# Header files
set(HEADERS
    extension.h
    smsdk_config.h
)

# Create the extension library
add_library(http_mongodb_ext SHARED ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(http_mongodb_ext ${CURL_LIBRARIES})

# Platform-specific linking
if(WIN32)
    target_link_libraries(http_mongodb_ext
        ${HL2SDK_PATH}/lib/public/tier0.lib
        ${HL2SDK_PATH}/lib/public/tier1.lib
        ${HL2SDK_PATH}/lib/public/vstdlib.lib
    )
    set_target_properties(http_mongodb_ext PROPERTIES
        OUTPUT_NAME "http_mongodb.ext"
        SUFFIX ".dll"
    )
else()
    target_link_libraries(http_mongodb_ext
        ${HL2SDK_PATH}/lib/linux/tier1_i486.a
        ${HL2SDK_PATH}/lib/linux/mathlib_i486.a
        dl
    )
    set_target_properties(http_mongodb_ext PROPERTIES
        OUTPUT_NAME "http_mongodb.ext"
        SUFFIX ".so"
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
    )
endif()

# Set output directory
set_target_properties(http_mongodb_ext PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy built extension to main bin directory as standard name
add_custom_command(TARGET http_mongodb_ext POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/bin/libhttp_mongodb.ext.so
        ${CMAKE_SOURCE_DIR}/bin/http_mongodb.ext.so
    COMMENT "Copying complete extension to main bin directory as http_mongodb.ext.so"
)
