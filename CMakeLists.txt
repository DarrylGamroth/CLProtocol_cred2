cmake_minimum_required(VERSION 3.10)

project(clprotocol_stub LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_SUFFIX "cred2" CACHE STRING "Library suffix for CLProtocol stub")

add_library(CLProtocol SHARED src/clprotocol_cred2.cpp)
target_include_directories(CLProtocol PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/genicam_include
)
target_compile_definitions(CLProtocol PRIVATE CLPROTOCOL_EXPORTS)
set_target_properties(CLProtocol PROPERTIES OUTPUT_NAME "CLProtocol_${LIB_SUFFIX}")

add_custom_target(embed
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_embed_xml.sh
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts
  COMMENT "Regenerate embedded C-RED2 GenApi XML header"
)

install(TARGETS CLProtocol LIBRARY DESTINATION lib)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/share/C-RED2_GenApi.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/share/C-RED2_SFNC_MAPPING.md
  ${CMAKE_CURRENT_SOURCE_DIR}/include/clprotocol_cred2_xml.h
  DESTINATION share
)

install(PROGRAMS
  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_embed_xml.sh
  DESTINATION bin
)
