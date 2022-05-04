cmake_minimum_required(VERSION 3.9)


#include_directories(src/mauiman)

add_subdirectory(mauiman) #add mauiman libs

#add_subdirectory(modules)
#add_library(MauiMan2 INTERFACE)
#add_library(MauiMan2::MauiMan2 ALIAS MauiMan2)
#target_link_libraries(MauiMan2
#  INTERFACE
#    MauiMan:MauiMan
#)
add_subdirectory(app)
