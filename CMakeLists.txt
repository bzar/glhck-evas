cmake_minimum_required(VERSION 2.6)
project(glhck-evas)

include("cmake/subproject.cmake")

set(GLHCK_BUILD_EXAMPLES OFF CACHE BOOL "Skip GLHCK examples")
SET(GLFW_BUILD_EXAMPLES 0 CACHE BOOL "Don't build examples for GLFW")
SET(GLFW_BUILD_TESTS 0 CACHE BOOL "Don't build tests for GLFW")

add_subdirectory(lib)
find_package(PkgConfig REQUIRED)
#add_definitions(-DEFL_EO_API_SUPPORT)
pkg_check_modules(EFL REQUIRED evas ecore eina)
pkg_check_modules(Elementary REQUIRED elementary)

include_directories(
    ${kazmath_SOURCE_DIR}/src
    ${glhck_SOURCE_DIR}/include
    ${glfwhck_SOURCE_DIR}/include
    ${GLFW_SOURCE_DIR}/include
    ${EFL_INCLUDE_DIRS}
    ${Elementary_INCLUDE_DIRS}
)

add_executable(simple-render simple-render.c)
target_link_libraries(simple-render glfw glfwhck glhck ${GLHCK_LIBRARIES} ${GLFW_LIBRARIES} ${EFL_LIBRARIES})

add_executable(mouse-events mouse-events.c)
target_link_libraries(mouse-events glfw glfwhck glhck ${GLHCK_LIBRARIES} ${GLFW_LIBRARIES} ${EFL_LIBRARIES})

add_executable(elementary-ui elementary-ui.c)
target_link_libraries(elementary-ui glfw glfwhck glhck ${GLHCK_LIBRARIES} ${GLFW_LIBRARIES} ${EFL_LIBRARIES} ${Elementary_LIBRARIES})

