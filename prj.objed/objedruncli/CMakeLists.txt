project(objedruncli)

set(objedruncli_HDRS
  src/objedruncli.h
  src/objedrunutils.h)

set(objedruncli_SRCS 
  src/main.cpp
  src/objedruncli.cpp
  src/objedrunutils.cpp)

add_executable(objedruncli ${objedruncli_SRCS} ${objedruncli_HDRS})

set_target_properties(objedruncli PROPERTIES AUTOMOC TRUE)

target_link_libraries(objedruncli objedutils)

