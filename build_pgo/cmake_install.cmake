# Install script for directory: C:/Users/offan/Downloads/hcle_py_cpp

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/hcle")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/hcle_py" TYPE MODULE FILES "C:/Users/offan/Downloads/hcle_py_cpp/build_pgo/Debug/_hcle_py_d.cp312-win_amd64.pyd")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/hcle_py" TYPE MODULE FILES "C:/Users/offan/Downloads/hcle_py_cpp/build_pgo/Release/_hcle_py.cp312-win_amd64.pyd")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/hcle_py" TYPE MODULE FILES "C:/Users/offan/Downloads/hcle_py_cpp/build_pgo/MinSizeRel/_hcle_py.cp312-win_amd64.pyd")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/hcle_py" TYPE MODULE FILES "C:/Users/offan/Downloads/hcle_py_cpp/build_pgo/RelWithDebInfo/_hcle_py.cp312-win_amd64.pyd")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/hcle_py" TYPE FILE FILES
      "C:/dev/vcpkg/installed/x64-windows/debug/bin/python312_d.dll"
      "C:/dev/vcpkg/installed/x64-windows/debug/bin/SDL2d.dll"
      "C:/dev/vcpkg/installed/x64-windows/debug/bin/opencv_imgproc4d.dll"
      "C:/dev/vcpkg/installed/x64-windows/debug/bin/opencv_core4d.dll"
      )
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/hcle_py" TYPE FILE FILES
      "C:/dev/vcpkg/installed/x64-windows/bin/python312.dll"
      "C:/dev/vcpkg/installed/x64-windows/bin/SDL2.dll"
      "C:/dev/vcpkg/installed/x64-windows/bin/opencv_imgproc4.dll"
      "C:/dev/vcpkg/installed/x64-windows/bin/opencv_core4.dll"
      )
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/hcle_py" TYPE FILE FILES
      "C:/dev/vcpkg/installed/x64-windows/bin/python312.dll"
      "C:/dev/vcpkg/installed/x64-windows/bin/SDL2.dll"
      "C:/dev/vcpkg/installed/x64-windows/bin/opencv_imgproc4.dll"
      "C:/dev/vcpkg/installed/x64-windows/bin/opencv_core4.dll"
      )
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/hcle_py" TYPE FILE FILES
      "C:/dev/vcpkg/installed/x64-windows/bin/python312.dll"
      "C:/dev/vcpkg/installed/x64-windows/bin/SDL2.dll"
      "C:/dev/vcpkg/installed/x64-windows/bin/opencv_imgproc4.dll"
      "C:/dev/vcpkg/installed/x64-windows/bin/opencv_core4.dll"
      )
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/offan/Downloads/hcle_py_cpp/build_pgo/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/offan/Downloads/hcle_py_cpp/build_pgo/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
