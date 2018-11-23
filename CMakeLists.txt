cmake_minimum_required(VERSION 2.8.1) 
find_package(Omegalib)

# Set the module name here
SET(MODULE_NAME cavegvdb)

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

include_directories(
        .
        ${OMEGA_INCLUDE_DIRS}
        /home/toand/git/omegalib/build/src/omicron/omicron/include
)

message(${CMAKE_SOURCE_DIR})
add_definitions(-DOMEGALIB_MODULE)

# Set module name and source files here
add_library(${MODULE_NAME} MODULE 
                app.h
                app.cpp
		cavegvdb.cpp 
        )

# Set the module library dependencies here
target_link_libraries(${MODULE_NAME}
        stdc++
        ${OMEGA_LIB}
        python2.7
        -lgvdb
        )

#------------------------------------------------------------------------------
# DO NOT MODIFY ANYTHING BELOW AFTER THIS LINE
set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")
if(WIN32)
        set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules SUFFIX ".pyd")
endif()