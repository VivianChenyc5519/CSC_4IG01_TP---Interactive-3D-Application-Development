# ----------------------------------------------------------------------------
# CMakeLists.txt
#
#  Created on: 13 Dec 2020
#      Author: Kiwon Um
#        Mail: kiwon.um@telecom-paris.fr
#
# Description: CMake configuration
#
# Copyright 2021-2025 Kiwon Um
#
# The copyright to the computer program(s) herein is the property of Kiwon Um,
# Telecom Paris, France. The program(s) may be used and/or copied only with
# the written permission of Kiwon Um or in accordance with the terms and
# conditions stipulated in the agreement/contract under which the program(s)
# have been supplied.
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10)

SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_definitions(_MY_OPENGL_IS_33_)

project(tpOpenGL)

add_executable(${PROJECT_NAME} main.cpp mesh.cpp camera.cpp)

target_sources(${PROJECT_NAME} PRIVATE dep/glad/src/gl.c)
target_include_directories(${PROJECT_NAME} PRIVATE dep/glad/include/)

add_subdirectory(dep/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

find_package(glfw3)
find_package(glad)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dep)
add_subdirectory(dep/glm)
target_link_libraries(${PROJECT_NAME} glm)

target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
  $<TARGET_FILE:${PROJECT_NAME}> 
  ${CMAKE_CURRENT_SOURCE_DIR}
)

  # Define a target to run the program under Valgrind
add_custom_target(valgrind
    COMMAND valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes
            --log-file=valgrind.log
            $<TARGET_FILE:tpOpenGL>
    DEPENDS tpOpenGL
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running tpOpenGL with Valgrind..."
)
