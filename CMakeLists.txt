cmake_minimum_required(VERSION 3.5)

project(boost_light)

add_executable(demo lsm.h lsg.h lpo.h main.cpp)
target_compile_features(demo PUBLIC cxx_std_17)



