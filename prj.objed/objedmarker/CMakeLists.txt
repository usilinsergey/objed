project(objedmarker)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(objedmarker_HDRS 
  src/objedmarker.h
  src/opendialog.h
  src/markupeditor.h
  src/objectsizedialog.h
  src/editorpropsdialog.h)

set(objedmarker_SRCS 
  src/main.cpp
  src/objedmarker.cpp
  src/opendialog.cpp
  src/markupeditor.cpp
  src/objectsizedialog.cpp
  src/editorpropsdialog.cpp)

if(WIN32)
  set(objedmarker_SRCS
    res/objedmarker.rc
    ${objedmarker_SRCS})
endif()

set(objedmarker_UIS 
  ui/objedmarker.ui
  ui/objectsizedialog.ui
  ui/editorpropsdialog.ui)

qt5_wrap_ui(objedmarker_UIHDRS 
  ${objedmarker_UIS})

qt5_add_resources(objedmarker_QRCSRCS
  res/objedmarker.qrc)

add_executable(objedmarker WIN32 ${objedmarker_HDRS} ${objedmarker_SRCS} ${objedmarker_QRCSRCS} ${objedmarker_UIHDRS})

set_target_properties(objedmarker PROPERTIES AUTOMOC TRUE)

target_link_libraries(objedmarker objedutils Qt5::WinMain)

qt5_use_modules(objedmarker Widgets Concurrent)