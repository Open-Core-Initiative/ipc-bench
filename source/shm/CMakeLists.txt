###########################################################
## TARGETS
###########################################################
add_executable(shm-server server.c)
add_executable(shm-client client.c)
add_executable(shm shm.c)

###########################################################
## COMMON
###########################################################
target_link_libraries(shm-server tunbench-common)
target_link_libraries(shm-client tunbench-common)
target_link_libraries(shm tunbench-common)
