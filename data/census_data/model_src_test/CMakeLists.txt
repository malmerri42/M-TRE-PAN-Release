cmake_minimum_required(VERSION 3.10)

# set the project name
project(M-TRE-PAN_Data VERSION 1.0
        LANGUAGES CXX)


# add the executable
add_executable(TestBuild
    ./main.cpp
    )

#specify c++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON )


#link package
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(TestBuild PUBLIC OpenMP::OpenMP_CXX)
endif()

#locations to look for includes
target_include_directories(TestBuild PUBLIC "${PROJECT_BINARY_DIR}" )

#add the mlpack subdir
find_package(mlpack REQUIRED PATHS "../../../source/mlpack" NO_DEFAULT_PATH)

#link library
find_library( ARMADILLO_LIB  armadillo )
#find_library( MLPACK_LIB  mlpack )

#library linked as private, because it is only ever used in implementation of the current target and not needed in any client targets
target_link_libraries(TestBuild PUBLIC ${ARMADILLO_LIB})
target_link_libraries(TestBuild PUBLIC mlpack)
#target_link_libraries(TestBuild PUBLIC ${MLPACK_LIB})

#include dir
target_include_directories(TestBuild PRIVATE mlpack)

#ignore fpermissive errors
target_compile_options(TestBuild PUBLIC -fpermissive)
