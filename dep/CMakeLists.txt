
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
add_subdirectory(GLFW)
add_subdirectory(anvil)

set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm PARENT_SCOPE)
set(ANVIL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/anvil/include PARENT_SCOPE)
