cmake_minimum_required(VERSION 3.16)
project(morphology_dilation LANGUAGES CXX)

add_executable(morphology_dilation main.cpp)
target_compile_features(morphology_dilation PRIVATE cxx_std_17)

include(CTest)
if(BUILD_TESTING)
  add_test(NAME dilation_demo COMMAND morphology_dilation --demo)
  add_test(NAME erosion_demo COMMAND morphology_dilation --demo --erode)
  add_test(NAME rounded_dilation_demo COMMAND morphology_dilation --demo --rounded)
endif()
