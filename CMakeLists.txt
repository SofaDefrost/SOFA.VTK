# CMakeLists.txt for SOFAVTK plugin
cmake_minimum_required(VERSION 3.12)
project(SOFAVTK VERSION 1.0 LANGUAGES CXX)

find_package(Sofa.Core REQUIRED)

sofa_find_package(VTK COMPONENTS
    CommonCore
    CommonDataModel
    IOGeometry
    IOLegacy
    IOPLY
    IOXML
REQUIRED)

set(HEADER_FILES
    src/SOFAVTK/config.h.in
    src/SOFAVTK/init.h

    src/SOFAVTK/PolyDataVTKLoader.h
    src/SOFAVTK/UnstructuredGridVTKLoader.h

    src/SOFAVTK/CellTypeName.h
    src/SOFAVTK/VTKtoSOFA.h
)

set(SOURCE_FILES
    src/SOFAVTK/init.cpp

    src/SOFAVTK/PolyDataVTKLoader.cpp
    src/SOFAVTK/UnstructuredGridVTKLoader.cpp

    src/SOFAVTK/CellTypeName.cpp
    src/SOFAVTK/VTKtoSOFA.cpp
)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC Sofa.Core)
target_link_libraries(${PROJECT_NAME} PRIVATE ${VTK_LIBRARIES})

message(STATUS "VTK_LIBRARIES: ${VTK_LIBRARIES}")

# vtk_module_autoinit is needed
vtk_module_autoinit(
  TARGETS ${PROJECT_NAME}
  MODULES ${VTK_LIBRARIES}
)

sofa_create_package_with_targets(
    PACKAGE_NAME ${PROJECT_NAME}
    PACKAGE_VERSION ${PROJECT_VERSION}
    TARGETS ${PROJECT_NAME} AUTO_SET_TARGET_PROPERTIES
    INCLUDE_SOURCE_DIR "src"
    INCLUDE_INSTALL_DIR ${PROJECT_NAME}
    RELOCATABLE "plugins"
    )
    
cmake_dependent_option(SOFAVTK_BUILD_TESTS "Compile the automatic tests" ON "SOFA_BUILD_TESTS OR NOT DEFINED SOFA_BUILD_TESTS" OFF)
if(SOFAVTK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
