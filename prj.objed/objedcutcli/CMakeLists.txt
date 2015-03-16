project(objedcutcli)

set(objedcutcli_HDRS
  src/objedcutcli.h
  src/objedcututils.h)

set(objedcutcli_SRCS 
  src/main.cpp
  src/objedcutcli.cpp
  src/objedcututils.cpp)

add_executable(objedcutcli ${objedcutcli_HDRS} ${objedcutcli_SRCS})

set_target_properties(objedcutcli PROPERTIES AUTOMOC TRUE)

target_link_libraries(objedcutcli objedutils)