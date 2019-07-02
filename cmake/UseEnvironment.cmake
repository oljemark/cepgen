set(LXPLUS_SRC_ENV "source ${CMAKE_SOURCE_DIR}/source-lxplus.sh")
set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wall -cpp")
if($ENV{HOSTNAME} MATCHES "^lxplus[0-9]+.cern.ch")
  set(IS_LXPLUS "yes")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic-errors -g")
else()
  message(STATUS "clang or gcc above 6.1 is required")
  if(IS_LXPLUS)
    message(STATUS "Compiling on LXPLUS. Did you properly source the environment variables? E.g.\n\n\t${LXPLUS_SRC_ENV}\n")
  endif()
  message(FATAL_ERROR "Please clean up this build environment, i.e.\n\trm -rf CMake*\nand try again...")
endif()
if(NOT CMAKE_VERSION VERSION_LESS 3.1)
  set(CMAKE_CXX_STANDARD 14)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()
if(IS_LXPLUS)
  set(BASE_DIR "/cvmfs/sft.cern.ch/lcg")
  list(APPEND CMAKE_PREFIX_PATH "${BASE_DIR}/external/CMake/2.8.9/Linux-i386/share/cmake-2.8/Modules")
  set(GSL_DIR "${BASE_DIR}/releases/GSL/2.5-32fc5/x86_64-centos7-gcc62-opt")
  set(HEPMC_DIR "${BASE_DIR}/releases/HepMC/2.06.09-0a23a/x86_64-centos7-gcc62-opt")
  set(LHAPDF_DIR "${BASE_DIR}/releases/MCGenerators/lhapdf/6.2.2-8a3e6/x86_64-centos7-gcc62-opt")
  set(PYTHIA8_DIR "${BASE_DIR}/releases/MCGenerators/pythia8/240p1-ecd34/x86_64-centos7-gcc62-opt")
  set(PYTHON_DIR "${BASE_DIR}/releases/Python/2.7.15-075d4/x86_64-centos7-gcc62-opt")
  set(PYTHON_LIBRARY "${PYTHON_DIR}/lib/libpython2.7.so")
  set(PYTHON_EXECUTABLE "${PYTHON_DIR}/bin/python")
  set(PYTHON_INCLUDE_DIR "${PYTHON_DIR}/include/python2.7")

  message(STATUS "Compiling on LXPLUS. Do not forget to source the environment variables!")
  message(STATUS "e.g. `${LXPLUS_SRC_ENV}`")
endif()
#--- searching for GSL
find_library(GSL_LIB gsl HINTS ${GSL_DIR} PATH_SUFFIXES lib REQUIRED)
find_library(GSL_CBLAS_LIB gslcblas HINTS ${GSL_DIR} PATH_SUFFIXES lib)
find_path(GSL_INCLUDE gsl HINTS ${GSL_DIR} PATH_SUFFIXES include)
include_directories(${GSL_INCLUDE})
#--- searching for LHAPDF
find_library(LHAPDF LHAPDF HINTS ${LHAPDF_DIR} PATH_SUFFIXES lib)
find_path(LHAPDF_INCLUDE LHAPDF HINTS ${LHAPDF_DIR} PATH_SUFFIXES include)
#--- searching for HepMC
find_library(HEPMC_LIB NAMES HepMC3 HepMC HINTS ${HEPMC_DIR} PATH_SUFFIXES lib)
find_path(HEPMC_INCLUDE NAMES HepMC3 HepMC HINTS ${HEPMC_DIR} PATH_SUFFIXES include)
#--- searching for Pythia 8
set(PYTHIA8_DIRS $ENV{PYTHIA8_DIR} ${PYTHIA8_DIR} /usr /usr/local /opt/pythia8)
find_library(PYTHIA8 pythia8 HINTS ${PYTHIA8_DIRS} PATH_SUFFIXES lib)
find_path(PYTHIA8_INCLUDE Pythia8 HINTS ${PYTHIA8_DIRS} PATH_SUFFIXES include include/Pythia8 include/pythia8)

message(STATUS "GSL found in ${GSL_LIB}")
list(APPEND CEPGEN_EXTERNAL_CORE_REQS ${GSL_LIB} ${GSL_CBLAS_LIB})
list(APPEND CEPGEN_EXTERNAL_STRF_REQS ${GSL_LIB} ${GSL_CBLAS_LIB})

if(LHAPDF)
  message(STATUS "LHAPDF found in ${LHAPDF}")
  list(APPEND CEPGEN_EXTERNAL_STRF_REQS ${LHAPDF})
  add_definitions(-DLIBLHAPDF)
  include_directories(${LHAPDF_INCLUDE})
endif()
find_package(PythonLibs 2.7)
find_library(MUPARSER muparser)
if(MUPARSER)
  message(STATUS "muParser found in ${MUPARSER}")
  list(APPEND CEPGEN_EXTERNAL_CORE_REQS ${MUPARSER})
  add_definitions(-DMUPARSER)
else()
  find_path(EXPRTK exprtk.hpp PATH_SUFFIXES include)
  if(EXPRTK)
    message(STATUS "exprtk found in ${EXPRTK}")
    add_definitions(-DEXPRTK)
    include_directories(${EXPRTK})
  endif()
endif()
set(ALPHAS_PATH ${PROJECT_SOURCE_DIR}/External)
file(GLOB alphas_sources ${ALPHAS_PATH}/alphaS.f)
if(alphas_sources)
  message(STATUS "alphaS evolution found in ${alphas_sources}")
  add_definitions(-DALPHA_S)
endif()

