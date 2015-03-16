project(objedtraincli)

set(objedtraincli_HDRS
  src/objedtraincli.h
  src/objedtrainutils.h
  src/datasetproc.h
  src/trainscproc.h)

set(objedtraincli_SRCS 
  src/main.cpp
  src/realadaboost.cpp
  src/objedtraincli.cpp
  src/objedtrainutils.cpp
  src/datasetproc.cpp
  src/trainscproc.cpp
  src/realadaboost.cpp)

add_executable(objedtraincli ${objedtraincli_HDRS} ${objedtraincli_SRCS})

set_target_properties(objedtraincli PROPERTIES AUTOMOC TRUE)

target_link_libraries(objedtraincli objedutils)