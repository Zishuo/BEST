cmake_minimum_required(VERSION 2.8)
project(BEST)
set(CMAKE_CXX_COMPILER /etc/alternatives/g++)
set(lp_solve /usr/local/lib/liblpsolve55.a)
#set(libboost_program_options /usr/lib/x86_64-linux-gnu/libboost_program_options.a)
#set(libboost_date_time /usr/lib/x86_64-linux-gnu/libboost_date_time.a)
set(executable_output_path ${project_binary_dir}/bin)
set(library_output_path ${project_binary_dir}/lib)
set(CMAKE_CXX_FLAGS "-std=c++0x")
find_package(Boost 1.55.0 COMPONENTS program_options date_time)

include_directories(./)
aux_source_directory(. src_list)
add_executable(BEST ${src_list})

if(UNIX)
	target_link_libraries(BEST 
		#${libboost_program_options} 
		#${libboost_date_time} 
		${Boost_LIBRARIES}
		pthread 
		${lp_solve}
		dl
		)
elseif(WIN32)
elseif(APPLE)
endif(UNIX)

message(STATUS "c++ flag : "${CMAKE_CXX_FLAGS}) 
message(STATUS "this is source dir "${project_source_dir})
message(STATUS "this is binary dir "${project_binary_dir})
