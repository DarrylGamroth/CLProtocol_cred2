cmake_minimum_required(VERSION 3.10)

project(clprotocol_stub LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_SUFFIX "cred2" CACHE STRING "Library suffix for CLProtocol stub")

# CLProtocol installation layout follows GenICam CLProtocol standard v1.2 (page 4):
# - XML files at the CLProtocol root location
# - protocol driver libraries in OS-specific subdirectories
if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_CLPROTOCOL_PLATFORM_SUBDIR_DEFAULT "Win64_x64")
  else()
    set(_CLPROTOCOL_PLATFORM_SUBDIR_DEFAULT "Win32_i86")
  endif()
else()
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_CLPROTOCOL_PLATFORM_SUBDIR_DEFAULT "Linux64_x64")
  else()
    set(_CLPROTOCOL_PLATFORM_SUBDIR_DEFAULT "Linux32_i86")
  endif()
endif()
set(CLPROTOCOL_PLATFORM_SUBDIR "${_CLPROTOCOL_PLATFORM_SUBDIR_DEFAULT}" CACHE STRING
    "OS-specific subdirectory used for CLProtocol driver libraries")

add_library(CLProtocol SHARED src/clprotocol_cred2.cpp)
target_include_directories(CLProtocol PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/genicam_include
)
target_compile_definitions(CLProtocol PRIVATE
  CLPROTOCOL_EXPORTS
  CLP_LIB_SUFFIX="${LIB_SUFFIX}"
  CLP_PLATFORM_SUBDIR="${CLPROTOCOL_PLATFORM_SUBDIR}"
)
set_target_properties(CLProtocol PROPERTIES OUTPUT_NAME "CLProtocol_${LIB_SUFFIX}")

add_custom_target(embed
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_embed_xml.sh
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts
  COMMENT "Regenerate embedded C-RED2 GenApi XML header"
)

install(TARGETS CLProtocol
  LIBRARY DESTINATION ${CLPROTOCOL_PLATFORM_SUBDIR}
  RUNTIME DESTINATION ${CLPROTOCOL_PLATFORM_SUBDIR}
  ARCHIVE DESTINATION ${CLPROTOCOL_PLATFORM_SUBDIR}
)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/share/C-RED2_GenApi.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/share/C-RED2_SFNC_MAPPING.md
  ${CMAKE_CURRENT_SOURCE_DIR}/share/CLPROTOCOL_v1_2_COMPLIANCE.md
  ${CMAKE_CURRENT_SOURCE_DIR}/include/clprotocol_cred2_xml.h
  DESTINATION .
)

install(PROGRAMS
  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_embed_xml.sh
  DESTINATION bin
)
