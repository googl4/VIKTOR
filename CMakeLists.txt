cmake_minimum_required(VERSION 3.9)

project(
	ViktorTest
	VERSION 1.0
	DESCRIPTION "VIKTOR Test"
	LANGUAGES C CXX
)

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -march=haswell")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=haswell")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_library(viktor STATIC src/viktor.c include/viktor.h)
target_include_directories(viktor PUBLIC include lib/volk)

add_library(volk STATIC lib/volk/volk.c lib/volk/volk.h)
target_include_directories(volk PUBLIC lib/volk)

add_library(inih STATIC lib/inih/ini.c lib/inih/ini.h)
target_include_directories(inih PUBLIC lib/inih)

add_executable(vrk src/test.cpp)
target_include_directories(vrk PUBLIC include lib lib/inih)
target_link_libraries(vrk PUBLIC viktor volk inih gdi32 winmm avrt -static)

#set_target_properties(vrk PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
#set_target_properties(viktor PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
#set_target_properties(volk PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)

#target_link_options(vrk PUBLIC -s)

target_compile_definitions(viktor PRIVATE "VK_USE_PLATFORM_WIN32_KHR")
target_compile_definitions(volk PRIVATE "VK_USE_PLATFORM_WIN32_KHR")
target_compile_definitions(vrk PRIVATE "VK_USE_PLATFORM_WIN32_KHR")
