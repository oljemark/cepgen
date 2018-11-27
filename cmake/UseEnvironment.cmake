set(LXPLUS_GCC_ENV "source /afs/cern.ch/sw/lcg/external/gcc/4.9.3/x86_64-slc6-gcc49-opt/setup.sh")
set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wall -cpp")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.7)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic-errors -g")
else()
  message(STATUS "clang or gcc above 4.7 is required")
  if($ENV{HOSTNAME} MATCHES "^lxplus[0-9]+.cern.ch")
    message(STATUS "Compiling on LXPLUS. Did you properly source the environment variables? E.g.\n\n\t${LXPLUS_GCC_ENV}\n")
  endif()
  message(FATAL_ERROR "Please clean up this build environment, i.e.\n\trm -rf CMake*\nand try again...")
endif()
if(NOT CMAKE_VERSION VERSION_LESS 3.1)
  set(CMAKE_CXX_STANDARD 11)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if($ENV{HOSTNAME} MATCHES "^lxplus[0-9]+.cern.ch")
  set(BASE_DIR "/afs/cern.ch/sw/lcg/external")
  list(APPEND CMAKE_PREFIX_PATH "${BASE_DIR}/CMake/2.8.9/Linux-i386/share/cmake-2.8/Modules")
  set(GSL_DIR "${BASE_DIR}/GSL/2.2.1/x86_64-slc6-gcc48-opt")
  set(HEPMC_DIR "${BASE_DIR}/HepMC/2.06.08/x86_64-slc6-gcc48-opt")
  #set(LHAPDF_DIR "${BASE_DIR}/MCGenerators/lhapdf/5.8.9/x86_64-slc6-gcc46-opt")
  set(LHAPDF_DIR "${BASE_DIR}/MCGenerators_lcgcmt67c/lhapdf/6.2.0/x86_64-slc6-gcc48-opt")
  set(PYTHIA8_DIR "${BASE_DIR}/MCGenerators_lcgcmt67c/pythia8/230/x86_64-slc6-gcc48-opt")
  set(PYTHON_DIR "${BASE_DIR}/Python/2.7.4/x86_64-slc6-gcc48-opt")
  set(PYTHON_LIBRARY "${PYTHON_DIR}/lib/libpython2.7.so")
  set(PYTHON_EXECUTABLE "${PYTHON_DIR}/bin/python")
  set(PYTHON_INCLUDE_DIR "${PYTHON_DIR}/include/python2.7")
  set(ROOTSYS "${BASE_DIR}/../app/releases/ROOT/6.06.08/x86_64-slc6-gcc49-opt/root")

  message(STATUS "Compiling on LXPLUS. Do not forget to source the environment variables!")
  message(STATUS "e.g. `${LXPLUS_GCC_ENV}`")
  #--- searching for GSL
  find_library(GSL_LIB gsl HINTS ${GSL_DIR} PATH_SUFFIXES lib REQUIRED)
  find_library(GSL_CBLAS_LIB gslcblas HINTS ${GSL_DIR} PATH_SUFFIXES lib)
  #--- searching for LHAPDF
  find_library(LHAPDF LHAPDF HINTS ${LHAPDF_DIR} PATH_SUFFIXES lib)
  find_path(LHAPDF_INCLUDE LHAPDF HINTS ${LHAPDF_DIR} PATH_SUFFIXES include)
  #--- searching for HepMC
  find_library(HEPMC_LIB HepMC HINTS ${HEPMC_DIR} PATH_SUFFIXES lib)
  find_path(HEPMC_INCLUDE HepMC HINTS ${HEPMC_DIR} PATH_SUFFIXES include)
else()
  find_library(GSL_LIB gsl REQUIRED)
  find_library(GSL_CBLAS_LIB gslcblas)
  find_library(LHAPDF LHAPDF)
  find_path(LHAPDF_INCLUDE LHAPDF)
  find_library(HEPMC_LIB HepMC)
  find_path(HEPMC_INCLUDE HepMC)
endif()
#--- searching for Pythia 8
set(PYTHIA8_DIRS $ENV{PYTHIA8_DIR} ${PYTHIA8_DIR} /usr /usr/local /opt/pythia8)
find_library(PYTHIA8 pythia8 HINTS ${PYTHIA8_DIRS} PATH_SUFFIXES lib)
find_path(PYTHIA8_INCLUDE Pythia8 HINTS ${PYTHIA8_DIRS} PATH_SUFFIXES include include/Pythia8 include/pythia8)

message(STATUS "GSL found in ${GSL_LIB}")
list(APPEND CEPGEN_EXTERNAL_IO_REQS ${GSL_LIB} ${GSL_CBLAS_LIB})
list(APPEND CEPGEN_EXTERNAL_CORE_REQS ${GSL_LIB} ${GSL_CBLAS_LIB})
list(APPEND CEPGEN_EXTERNAL_STRF_REQS ${GSL_LIB} ${GSL_CBLAS_LIB})

if(LHAPDF)
  message(STATUS "LHAPDF found in ${LHAPDF}")
  list(APPEND CEPGEN_EXTERNAL_STRF_REQS ${LHAPDF})
  add_definitions(-DLIBLHAPDF)
  include_directories(${LHAPDF_INCLUDE})
endif()
find_package(PythonLibs 2.7)
if(PYTHONLIBS_FOUND)
  list(APPEND CEPGEN_EXTERNAL_CARDS_REQS ${PYTHON_LIBRARIES})
  add_definitions(-DPYTHON)
  message(STATUS "Python v${PYTHONLIBS_VERSION_STRING} found")
  include_directories(${PYTHON_INCLUDE_DIRS})
endif()
if(PYTHIA8)
  message(STATUS "Pythia 8 found in ${PYTHIA8}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-misleading-indentation")
  list(APPEND CEPGEN_EXTERNAL_HADR_REQS ${PYTHIA8} dl)
  add_definitions(-DPYTHIA8)
  include_directories(${PYTHIA8_INCLUDE})
endif()
if(HEPMC_LIB)
  message(STATUS "HepMC found in ${HEPMC_INCLUDE}")
  list(APPEND CEPGEN_EXTERNAL_IO_REQS ${HEPMC_LIB})
  add_definitions(-DLIBHEPMC)
  include_directories(${HEPMC_INCLUDE})
endif()
find_library(MUPARSER muparser)
if(MUPARSER)
  message(STATUS "muParser found in ${MUPARSER}")
  list(APPEND CEPGEN_EXTERNAL_CORE_REQS ${MUPARSER})
  add_definitions(-DMUPARSER)
endif()
set(ALPHAS_PATH ${PROJECT_SOURCE_DIR}/External)
file(GLOB alphas_sources ${ALPHAS_PATH}/alphaS.f)
if(alphas_sources)
  message(STATUS "alpha(s) evolution found in ${alphas_sources}")
  add_definitions(-DALPHA_S)
endif()

