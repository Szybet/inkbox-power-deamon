cmake_minimum_required(VERSION 3.0.0)
project(inkbox-power-deamon VERSION 0.1.0)

include(CTest)
enable_testing()

# https://stackoverflow.com/questions/1662909/undefined-reference-to-pthread-create-in-linux
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# https://stackoverflow.com/questions/3201154/automatically-add-all-files-in-a-folder-to-a-target-using-cmake
file(GLOB fileSrc
     "src/*.h"
     "src/*.cpp"
)

add_executable(inkbox-power-deamon ${fileSrc})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
