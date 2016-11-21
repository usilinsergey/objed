project(visualizer)

set(visualizer_SRCS visualizer.h visualizer.cpp)

add_executable(visualizer ${visualizer_SRCS})

target_link_libraries(visualizer objedutils)