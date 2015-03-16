project(objed)

set(OBJED_MAJOR_VERSION "1"   CACHE STRING "Major part of Objed version number")
set(OBJED_MINOR_VERSION "4"   CACHE STRING "Minor part of Objed version number")

set(objed_PUBLIC_HDRS
  objed.h
  objedutils.h
  simplest.h)

set(objed_HDRS
  src/imagepool.h
  src/imgutils.h
  src/maxcl.h
  src/linearcl.h
  src/additivecl.h
  src/cascadecl.h
  src/parallelcl.h
  src/treecl.h
  src/haar1stumpcl.h
  src/haar2stumpcl.h
  src/haar3stumpcl.h
  src/haar1pwcl.h
  src/haar2pwcl.h
  src/haar3pwcl.h
  src/meancl.h
  src/roicl.h
  src/simpledet.h
  src/yscaledet.h
  src/multidet.h
  src/lazydet.h)

set(objed_SRCS
  src/objed.cpp
  src/objedutils.cpp
  src/imagepool.cpp
  src/imgutils.cpp
  src/maxcl.cpp
  src/linearcl.cpp
  src/additivecl.cpp
  src/cascadecl.cpp
  src/parallelcl.cpp
  src/treecl.cpp
  src/haar1stumpcl.cpp
  src/haar2stumpcl.cpp
  src/haar3stumpcl.cpp
  src/haar1pwcl.cpp
  src/haar2pwcl.cpp
  src/haar3pwcl.cpp
  src/meancl.cpp
  src/roicl.cpp
  src/simpledet.cpp
  src/yscaledet.cpp
  src/multidet.cpp
  src/lazydet.cpp)

add_library(objed STATIC ${objed_PUBLIC_HDRS} ${objed_HDRS} ${objed_SRCS})
target_link_libraries(objed jsoncpp ${OpenCV_LIBS})  
