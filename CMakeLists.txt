cmake_minimum_required(VERSION 3.14)
project(ChatIdShowcasePlugin LANGUAGES CXX)

if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/LogosModule.cmake")
    include(cmake/LogosModule.cmake)
else()
    message(FATAL_ERROR "LogosModule.cmake not found — set LOGOS_MODULE_BUILDER_ROOT")
endif()

logos_module(
    NAME chat_id_showcase
    SOURCES
        src/ChatIdShowcasePlugin.h
        src/ChatIdShowcasePlugin.cpp
    INCLUDE_DIRS
        src
)
