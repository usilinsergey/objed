project(cascade2tree)

set(cascade2tree_SRCS cascade2tree.h cascade2tree.cpp)

add_executable(cascade2tree ${cascade2tree_SRCS})

target_link_libraries(cascade2tree objedutils)