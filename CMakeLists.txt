cmake_minimum_required(VERSION 3.22.1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

project(three_d_renderer)

message(${CMAKE_BUILD_TYPE})

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(configure_project)

    set(FILES)
    list(APPEND FILES 
        src/main.cpp
    )

    add_executable(${PROJECT_NAME} 
        MACOSX_BUNDLE
        ${FILES}
    )

    target_include_directories(${PROJECT_NAME} 
        PUBLIC src
    )

endfunction()


configure_project()
