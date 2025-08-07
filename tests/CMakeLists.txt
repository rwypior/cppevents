cmake_minimum_required (VERSION 3.26)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("eventsTest")

add_executable(eventsTest)
set_property(TARGET eventsTest PROPERTY CXX_STANDARD 17)

target_sources(eventsTest PRIVATE "main.cpp" "freefunctiontests.cpp" "common.h" "lambdatest.cpp" "memberfunctions.cpp")

find_package(Catch2 3.6.0 REQUIRED)

target_include_directories(eventsTest PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(eventsTest PRIVATE events)
target_link_libraries(eventsTest PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(eventsTest)
