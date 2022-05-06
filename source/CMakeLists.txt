cmake_minimum_required(VERSION 3.10)

# set the project name
project(M-TRE-PAN VERSION 1.0
        LANGUAGES CXX)


# add the executable
add_executable(MTREPAN 
    ./main.cpp
    ./Data_List.cpp
    ./Constraint.cpp
    ./Tree_Builder.cpp
    ./Deci_Tree_Builder.cpp
    ./Leaf_Node.cpp
    ./Tree_Node.cpp
    ./Deci_Tree_Node.cpp
    ./Oracle.cpp
    ./Ann_Oracle.cpp
    ./Base_Tree_Node.cpp
    ./MyStats.cpp
    ./Circle_Oracle.cpp
    ./Branch_Builder.cpp
    ./Sine_Oracle.cpp
    ./Local_Exp_Visitor.cpp
    )

#specify c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON )

#specify flags for debug
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#link packages
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(MTREPAN PUBLIC OpenMP::OpenMP_CXX)
endif()
#find_package(mlpack REQUIRED)
#target_link_libraries(MTREPAN PUBLIC mlpack)

#locations to look for includes
target_include_directories(MTREPAN PUBLIC "${PROJECT_BINARY_DIR}" )


#link library
find_library( ARMADILLO_LIB  armadillo )
find_library( MLPACK_LIB  mlpack )

#library linked as private, because it is only ever used in implementation of the current target and not needed in any client targets
target_link_libraries(MTREPAN PUBLIC ${ARMADILLO_LIB})
target_link_libraries(MTREPAN PUBLIC ${MLPACK_LIB})
