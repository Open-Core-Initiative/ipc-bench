###########################################################
## CONFIGURATION
###########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_compile_options(-g -O3 -Wall -Werror)

# Apple's C standard library already contains the math functions
if(APPLE)
	add_compile_options(-std=c11)
else()
	add_compile_options(-std=gnu11)
	link_libraries(m)
endif()

link_libraries(pthread)

###########################################################
## DEPENDENCIES
###########################################################

find_package(PkgConfig REQUIRED)

## ZMQ

pkg_check_modules(ZMQ libzmq)

###########################################################
## SUBDIRECTORIES
###########################################################

add_subdirectory(common)
add_subdirectory(shm)
