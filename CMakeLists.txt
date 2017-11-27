cmake_minimum_required(VERSION 2.8)
project (4d_explore)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    include(CheckCXXCompilerFlag)
    
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
    
    if(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    elseif(COMPILER_SUPPORTS_CXX0X)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
endif()

add_subdirectory   (dep/anvil "${CMAKE_CURRENT_BINARY_DIR}/anvil")


include_directories(${Anvil_SOURCE_DIR}/include
                    ${glm_SOURCE_DIR}/glm
                    ${4d_explore_SOURCE_DIR}/include)

# Include the Vulkan header.
if (WIN32)
    include_directories($ENV{VK_SDK_PATH}/Include
                        $ENV{VULKAN_SDK}/Include)
    
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            link_directories   ($ENV{VK_SDK_PATH}/Bin
                                $ENV{VK_SDK_PATH}/Lib
                                $ENV{VULKAN_SDK}/Bin
                                $ENV{VULKAN_SDK}/Lib)
    else()
            link_directories   ($ENV{VK_SDK_PATH}/Bin32
                                $ENV{VK_SDK_PATH}/Lib32
                                $ENV{VULKAN_SDK}/Bin32
                                $ENV{VULKAN_SDK}/Lib32)
    endif()
else()
    include_directories($ENV{VK_SDK_PATH}/x86_64/include
                        $ENV{VULKAN_SDK}/include
                        $ENV{VULKAN_SDK}/x86_64/include)
    link_directories   ($ENV{VK_SDK_PATH}/x86_64/lib
                        $ENV{VULKAN_SDK}/lib
                        $ENV{VULKAN_SDK}/x86_64/lib)
endif()

# Create the 4d_explore project.
add_executable (4d_explore src/app.h
                           src/app.cpp
                           src/matrix.h
                           src/matrix.cpp
                           src/camera.h
                           src/camera.cpp
                           src/main.cpp)

# Add linking dependencies for the example projects
add_dependencies     (4d_explore Anvil)

if (WIN32)
    target_link_libraries(4d_explore Anvil)
else()
    target_link_libraries(4d_explore Anvil dl)
endif()
