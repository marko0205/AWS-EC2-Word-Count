# Set minimum required version for CMake
cmake_minimum_required(VERSION 3.10)

# Define the project name 
project(word-count)

# Define C standard
set(CMAKE_C_STANDARD 99)

# Require external dependencies
find_package(MPI REQUIRED)

# Include MPI directories
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

# Define libraries
add_library(file_utils STATIC src/file_utils.c)
target_include_directories(file_utils PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(frequency_list STATIC src/frequency_list.c)
target_include_directories(frequency_list PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(schedule STATIC src/schedule.c)
target_include_directories(schedule PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(mpi_utils STATIC src/mpi_utils.c)
target_include_directories(mpi_utils PUBLIC ${PROJECT_SOURCE_DIR}/include)


# Define the executable of the project
add_executable(${PROJECT_NAME} src/word-count.c)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE file_utils frequency_list schedule mpi_utils ${MPI_C_LIBRARIES})