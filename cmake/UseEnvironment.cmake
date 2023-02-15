if(CMAKE_VERSION VERSION_GREATER 3.1)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_C_STANDARD 11)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
endif()
if(CMAKE_VERSION VERSION_GREATER 2.8)
  find_program(CCACHE ccache HINTS $ENV{CCACHE_PATH} PATH_SUFFIXES bin)
  if(CCACHE)
    message(STATUS "CCache found in ${CCACHE}")
    if(IS_LXPLUS)
      set(CCACHE_DIR "${CMAKE_SOURCE_DIR}/ccache")
      file(MAKE_DIRECTORY ${CCACHE_DIR})
      message(STATUS "CCache output directory set to ${CCACHE_DIR}")
    endif(IS_LXPLUS)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE}) # Less useful to do it for linking, see edit2
  endif(CCACHE)
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O")
set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wall -cpp")
#--- ensure a proper version of the compiler is found
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic-errors -g")
else()
  message(STATUS "clang or gcc above 7 is required")
  if(IS_LXPLUS)
    message(STATUS "Compiling on LXPLUS. Did you properly source the environment variables? E.g.\n\n\tsource ${LXPLUS_ENV}\n")
  endif()
  message(FATAL_ERROR "Please clean up this build environment, i.e.\n\trm -rf CMake*\nand try again...")
endif()
#--- set the default paths for external dependencies
if(IS_LXPLUS)
  list(APPEND CMAKE_PREFIX_PATH "$ENV{JSONMCPP_DIR}/lib64/cmake")
endif()
#--- searching for GSL
find_library(GSL_LIB gsl HINTS $ENV{GSL_DIR} PATH_SUFFIXES lib REQUIRED)
find_path(GSL_INCLUDE gsl HINTS $ENV{GSL_DIR} PATH_SUFFIXES include REQUIRED)
list(APPEND CEPGEN_CORE_EXT ${GSL_LIB})
#--- either use GSL's CBLAS or OpenBLAS implementation
find_library(OPENBLAS_LIB openblas HINTS $ENV{OPENBLAS_DIR} PATH_SUFFIXES lib)
if(OPENBLAS_LIB)
  message(STATUS "OpenBLAS found in ${OPENBLAS_LIB}")
  list(APPEND CEPGEN_CORE_EXT ${OPENBLAS_LIB})
else()
  find_library(GSL_CBLAS_LIB gslcblas HINTS $ENV{GSL_DIR} $ENV{GSL_CBLAS_DIR} PATH_SUFFIXES lib)
  if(NOT GSL_CBLAS_LIB)
    message(FATAL_ERROR "Neither OpenBLAS nor GSL's CBLAS library found.")
  endif()
  message(STATUS "GSL CBLAS found in ${GSL_CBLAS_LIB}")
  list(APPEND CEPGEN_CORE_EXT ${GSL_CBLAS_LIB})
endif()
include_directories(${GSL_INCLUDE})
#--- searching for ROOT
find_package(ROOT QUIET)
if(ROOT_FOUND)
  if(${ROOT_CXX_FLAGS} MATCHES "-std=c\\+\\+([0-9]+).*")
    string(SUBSTRING ${CMAKE_MATCH_1} 0 2 ROOT_CXX_STANDARD)
    if(${ROOT_CXX_STANDARD} VERSION_LESS 14)
      set(ROOT_CXX_STANDARD 14)
    endif()
  endif()
  if(IS_LXPLUS)
    #--- LXPLUS/CVMFS tweak for missing dependencies
    find_library(TBB tbb HINTS $ENV{TBB_DIR} PATH_SUFFIXES lib QUIET)
    if(TBB)
      list(APPEND ROOT_LIBRARIES ${TBB})
    endif()
    find_library(VDT vdt HINTS $ENV{VDT_DIR} PATH_SUFFIXES lib)
    if(VDT)
      list(APPEND ROOT_LIBRARIES ${VDT})
    endif()
    list(APPEND ROOT_INCLUDE_DIRS ${ROOT_INCLUDE_DIR})
  endif()
endif()
