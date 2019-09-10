cmake_minimum_required(VERSION 3.5)

project(boost_light)

add_executable(boost_light_test lsm.h lsg.h main.cpp)
target_compile_features(boost_light_test PUBLIC cxx_std_17)



