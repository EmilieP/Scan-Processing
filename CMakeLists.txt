cmake_minimum_required(VERSION 2.8)

PROJECT( Scan-Processing )

FIND_PACKAGE( OpenCV REQUIRED )

SET(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

file( GLOB SOURCES src/*.cpp )

foreach( src_file ${SOURCES} )
  get_filename_component( filename ${src_file} NAME_WE )
  add_executable( ${filename} ${src_file} )
  target_link_libraries( ${filename} ${OpenCV_LIBS} )
endforeach( src_file ${SOURCES} )