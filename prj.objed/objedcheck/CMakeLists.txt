project(objedcheck)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(objedcheck_HDRS 
  src/objedcheck.h
  src/imageviewdock.h)

set(objedcheck_SRCS 
  src/main.cpp
  src/objedcheck.cpp
  src/imageviewdock.cpp)

if(WIN32)
  set(objedcheck_SRCS
    res/objedcheck.rc
    ${objedcheck_SRCS})
endif()

set(objedcheck_UIS 
  ui/objedcheck.ui)

qt5_wrap_ui(objedcheck_UIHDRS 
  ${objedcheck_UIS})

qt5_add_resources(objedcheck_QRCSRCS
  res/objedcheck.qrc)

add_executable(objedcheck WIN32 ${objedcheck_HDRS} ${objedcheck_SRCS} ${objedcheck_QRCSRCS} ${objedcheck_UIHDRS})

set_target_properties(objedcheck PROPERTIES AUTOMOC TRUE)

target_link_libraries(objedcheck objedutils Qt5::WinMain)

qt5_use_modules(objedcheck Widgets)