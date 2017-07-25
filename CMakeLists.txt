cmake_minimum_required (VERSION 3.0.2) # 3.0.2 подходит
project (ase LANGUAGES CXX)
add_executable (ase ase.cpp)
install (TARGETS ase DESTINATION bin)
