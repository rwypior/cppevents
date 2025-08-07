cmake_minimum_required (VERSION 3.26)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_DEBUG_POSTFIX "d")

project ("events")
add_library(events INTERFACE)
add_library(events::events ALIAS events)

set_property(TARGET events PROPERTY CXX_STANDARD 17)

target_include_directories (events INTERFACE 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/events>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if(${BUILD_TESTING})
    message("Building unit tests")

    include(CTest)
    add_subdirectory ("tests")
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(ConfigPackageLocation lib/cmake/events)

configure_package_config_file(
        cmake/eventsConfig.cmake.in
	    "${CMAKE_CURRENT_BINARY_DIR}/events/eventsConfig.cmake"
	    INSTALL_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/events"
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/events/eventsConfigVersion.cmake"
        VERSION 1.0.0
        COMPATIBILITY AnyNewerVersion
)

install(
        TARGETS events EXPORT eventsTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
        DIRECTORY "include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        FILES_MATCHING 
        PATTERN "*.h"
        PATTERN "*.hpp"
)

install(EXPORT eventsTargets
        FILE eventsTargets.cmake
        NAMESPACE events::
        DESTINATION ${ConfigPackageLocation}
)

install(FILES 
            "${CMAKE_CURRENT_BINARY_DIR}/events/eventsConfig.cmake" 
            "${CMAKE_CURRENT_BINARY_DIR}/events/eventsConfigVersion.cmake"
        DESTINATION ${ConfigPackageLocation}
)

export(PACKAGE events)
