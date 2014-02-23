cmake_minimum_required(VERSION 2.6)
project(glhck-evas)

include("cmake/subproject.cmake")

set(GLHCK_BUILD_EXAMPLES OFF CACHE BOOL "Skip GLHCK examples")
SET(GLFW_BUILD_EXAMPLES 0 CACHE BOOL "Don't build examples for GLFW")
SET(GLFW_BUILD_TESTS 0 CACHE BOOL "Don't build tests for GLFW")

add_subdirectory(lib)
find_package(PkgConfig REQUIRED)

pkg_check_modules(EFL REQUIRED evas ecore eina ecore-evas)
include_directories(
    ${kazmath_SOURCE_DIR}/src
    ${glhck_SOURCE_DIR}/include
    ${GLFW_SOURCE_DIR}/include
    ${EFL_INCLUDE_DIRS}
)
add_executable(glhck-evas main.c)
message("GLHCK:" ${GLHCK_LIBRARIES})
message("GLFW:" ${GLFW_LIBRARIES})
message("EFL:" ${EFL_LIBRARIES})
target_link_libraries(glhck-evas glfw glhck ${GLHCK_LIBRARIES} ${GLFW_LIBRARIES} ${EFL_LIBRARIES})

