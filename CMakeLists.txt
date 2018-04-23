cmake_minimum_required(VERSION 3.2)
project (restan)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(.)
include_directories(peglib)

file(GLOB _SRC
    "*.cpp"
    "hmcmc/*.cpp"
    "pi/*.cpp"
    "parser/*.cpp"
    "utils/*.cpp"
    "data/*"
)

file(GLOB _MAIN
    "main/*.cpp"
)

file (GLOB _TEST
    "tests/*.cpp"
)

file(GLOB _GIBBS
	"gibbs/*.cpp"
)

add_executable(restan
  ${_MAIN}
)

add_executable(stan-tests
  ${_TEST}
)

add_executable(gibbs-test
  ${_GIBBS}
)

add_library(restan-common
  ${_SRC}
)

target_link_libraries(restan-common openblas adept)

target_link_libraries(restan restan-common openblas adept)
target_link_libraries(stan-tests restan-common openblas adept)
target_link_libraries(gibbs-test restan-common openblas adept)
