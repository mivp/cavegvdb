cmake_minimum_required(VERSION 2.8.1) 

project(stdapp)

# FIND OPENGL
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

include_directories(
        external/glm-0.9.4.0/
	external/glfw-3.0.3/include/GLFW/
	external/glew-1.9.0/include/
	../
	.
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include( ${CMAKE_MODULE_PATH}/Helpers.cmake )  
include( ${CMAKE_MODULE_PATH}/Packages.cmake )  

# Add GVDB
#
_add_package_GVDB()
include_directories (
        ${GVDB_INCLUDE_DIR}
)
link_directories(
        ${GVDB_LIB_DIR}
)

####################################################################################
# Add CUDA
#
_add_package_CUDA()


####################################################################################
# Compile PTX Files
#
file(GLOB CUDA_FILES RELATIVE "${BASE_DIRECTORY}" *.cu *.cuh)
message ( STATUS "Build CUDA kernels: ${CUDA_FILES}" )
_COMPILEPTX ( SOURCES ${CUDA_FILES} TARGET_PATH ${EXECUTABLE_OUTPUT_PATH} GENERATED CUDA_PTX GENPATHS CUDA_PTX_PATHS INCLUDE "${CMAKE_CURRENT_SOURCE_DIR},${GVDB_INCLUDE_DIR}" OPTIONS -arch=compute_30 -code=sm_30 --ptxas-options=-v -O3 --use_fast_math --maxrregcount=128 )

#####################################################################################
# Asset Path
#
if ( NOT DEFINED ASSET_PATH ) 
  get_filename_component ( _assets "${CMAKE_CURRENT_SOURCE_DIR}/../shared_assets" REALPATH )
  set ( ASSET_PATH ${_assets} CACHE PATH "Full path to gvdb/shared_assets/" )  
endif()


add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

# Compile external dependencies 
add_subdirectory (external)


set( srcs
        ../app.h
        ../app.cpp
        GLUtils.h
        GLUtils.cpp
        Camera.h 
        Camera.cpp
        main.cpp 
        )

add_executable(stdapp ${srcs} )
target_link_libraries(stdapp 
        -lgvdb
        ${OPENGL_LIBRARY}
	GLFW_303
	GLEW_190
)