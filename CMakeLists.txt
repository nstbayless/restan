cmake_minimum_required(VERSION 2.0)
project (restan)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(.)

file(GLOB _SRC
    "*.cpp"
    "hmcmc/*.cpp"
    "pi/*.cpp"
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
  ${_SRC}
  ${_MAIN}
)

add_executable(stan-tests
  ${_SRC}
  ${_TEST}
)

add_executable(gibbs-test
  ${_SRC}
  ${_GIBBS}
)

target_link_libraries(restan openblas adept)
target_link_libraries(stan-tests openblas adept)
target_link_libraries(gibbs-test openblas adept)
