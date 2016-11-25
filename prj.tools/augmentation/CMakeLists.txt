project(augmentation)

set(augmentation_SRCS 
  augmentation.h
  augmentation.cpp
  augmentation_utils.h
  augmentation_utils.cpp
  augmentation_gamma.h
  augmentation_gamma.cpp
  augmentation_rotate.h
  augmentation_rotate.cpp
)

add_executable(augmentation ${augmentation_SRCS})

target_link_libraries(augmentation objedutils)