cmake_minimum_required(VERSION 2.8.11)
cmake_policy(SET CMP0020 NEW)

project(objedutils)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/objedconf.h.cmakein
  ${CMAKE_CURRENT_BINARY_DIR}/objedconf.h)

set(objedutils_HDRS
  objedio.h
  objedsys.h
  objedexp.h
  objedimage.h
  objedtimer.h
  objedmarkup.h
  objedcompare.h
  objedconfig.h
  objedopenmp.h
  objedconsole.h)

set(objedutils_SRCS
  src/objedio.cpp
  src/objedsys.cpp
  src/objedexp.cpp
  src/objedimage.cpp
  src/objedtimer.cpp
  src/objedmarkup.cpp
  src/objedcompare.cpp
  src/objedconfig.cpp
  src/objedopenmp.cpp
  src/objedconsole.cpp)

add_library(objedutils STATIC ${objedutils_HDRS} ${objedutils_SRCS})
target_link_libraries(objedutils objed ${OpenCV_LIBS})
qt5_use_modules(objedutils Core Widgets)