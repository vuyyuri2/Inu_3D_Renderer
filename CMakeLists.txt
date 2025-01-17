cmake_minimum_required(VERSION 3.22.1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

project(three_d_renderer)

message(${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(configure_project)

    set(FILES)
    list(APPEND FILES 
        src/main.cpp

        src/windowing/window.cpp

        src/gfx/gfx.cpp
        src/gfx/opengl/glew.c
        src/gfx/online_renderer.cpp

        src/model_loading/gltf/gltf.cpp
        src/model_loading/image/stb_image.cpp
        src/model_loading/model_internal.cpp

        src/utils/general.cpp
        src/utils/inu_math.cpp
        src/utils/mats.cpp
        src/utils/quaternion.cpp
        src/utils/vectors.cpp
        src/utils/inu_time.cpp

        src/scene/scene.cpp
        src/scene/transform.cpp
        src/scene/camera.cpp

        src/animation/interpolation.cpp
        src/animation/animation_internal.cpp
    )

    add_executable(${PROJECT_NAME} 
        WIN32
        ${FILES}
    )

    target_include_directories(${PROJECT_NAME} 
        PUBLIC src
        PUBLIC src/gfx/opengl
        PUBLIC src/gfx/opengl/GL
    )

    target_link_libraries(${PROJECT_NAME}
        opengl32
    )

endfunction()


configure_project()
