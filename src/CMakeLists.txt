file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

file(GLOB IMAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/images/*.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/images/*.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/*.bmp
)

foreach(IMAGE ${IMAGES})
    get_filename_component(fname ${IMAGE} NAME)
    configure_file(${IMAGE} ${CMAKE_CURRENT_BINARY_DIR}/images/${fname} COPYONLY)
endforeach()

file(GLOB_RECURSE SHADER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/*.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/*.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/*.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.tese
    ${CMAKE_CURRENT_SOURCE_DIR}/*.tesc
)

source_group("Shaders" FILES ${SHADER_SOURCES})

if(WIN32)
    add_executable(4d_explore WIN32 ${SOURCES} ${SHADER_SOURCES})
    target_link_libraries(4d_explore ${WINLIBS})
else(WIN32)
    add_executable(4d_explore ${SOURCES})
    target_link_libraries(4d_explore ${CMAKE_THREAD_LIBS_INIT})
endif(WIN32)

foreach(SHADER_SOURCE ${SHADER_SOURCES})
    set(SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

    if(WIN32)
        get_filename_component(fname ${SHADER_SOURCE} NAME)
        add_custom_target(${fname}.spv
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_DIR} && 
            #$ENV{VK_SDK_PATH}/Bin/glslangValidator.exe -V ${SHADER_SOURCE} -o ${SHADER_DIR}/${fname}.spv
            copy ${SHADER_SOURCE} ${SHADER_DIR}/${fname}
            SOURCES ${SHADER_SOURCE}
        )
        ExternalTarget("Shaders" ${fname}.spv)
        add_dependencies(4d_explore ${fname}.spv)
    else(WIN32)
        get_filename_component(fname ${SHADER_SOURCE} NAME)
        add_custom_target(${fname}.spv
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_DIR} &&
            #$ENV{VULKAN_SDK}/bin/glslangValidator -V ${SHADER_SOURCE} -o ${SHADER_DIR}/${fname}.spv
            cp ${SHADER_SOURCE} ${SHADER_DIR}/${fname}
            SOURCES ${SHADER_SOURCE}
        )
        ExternalTarget("Shaders" ${fname}.spv)
        add_dependencies(4d_explore ${fname}.spv)
    endif(WIN32)

    # TODO: Build shaders on not windows
endforeach()

target_link_libraries(4d_explore ${ASSIMP_LIBRARIES} Vulkan::Vulkan glfw Anvil)
target_include_directories(4d_explore PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLM_INCLUDE_DIR}
  ${ANVIL_INCLUDE_DIR}
)

InternalTarget("" 4d_explore)
