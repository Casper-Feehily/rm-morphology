cmake_minimum_required(VERSION 3.16)
project(morphology_dilation LANGUAGES CXX)

add_executable(morphology_dilation main.cpp)
target_compile_features(morphology_dilation PRIVATE cxx_std_17)
