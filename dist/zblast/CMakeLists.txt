cmake_minimum_required(VERSION 3.20)
project(zblast_generator LANGUAGES NONE)

option(ZBLAST_GENERATE_VISITORS "Generate visitor sources from .zast files" ON)

if(NOT TARGET zblast::zblast)
    add_executable(zblast::zblast IMPORTED GLOBAL)
    set_target_properties(zblast::zblast PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/zblast${CMAKE_HOST_EXECUTABLE_SUFFIX}"
    )
endif()

include("cmake/zblast.cmake")