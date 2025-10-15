cmake_minimum_required(VERSION 3.12)
project(SOFA.VTK LANGUAGES CXX)

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
    src/sofa/vtk/config.h.in
    src/sofa/vtk/init.h

    src/sofa/vtk/BaseVTKLoader.h
    src/sofa/vtk/PolyDataVTKLoader.h
    src/sofa/vtk/UnstructuredGridVTKLoader.h

    src/sofa/vtk/CellTypeName.h
    src/sofa/vtk/VTKtoSOFA.h
)

set(SOURCE_FILES
    src/sofa/vtk/init.cpp

    src/sofa/vtk/BaseVTKLoader.cpp
    src/sofa/vtk/PolyDataVTKLoader.cpp
    src/sofa/vtk/UnstructuredGridVTKLoader.cpp

    src/sofa/vtk/CellTypeName.cpp
    src/sofa/vtk/VTKtoSOFA.cpp
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
    PACKAGE_VERSION ${Sofa_VERSION}
    TARGETS ${PROJECT_NAME} AUTO_SET_TARGET_PROPERTIES
    INCLUDE_SOURCE_DIR "src"
    INCLUDE_INSTALL_DIR ${PROJECT_NAME}
    EXAMPLE_INSTALL_DIR "examples"
    RELOCATABLE "plugins"
)

cmake_dependent_option(SOFA_VTK_BUILD_TESTS "Compile the automatic tests" ON "SOFA_BUILD_TESTS OR NOT DEFINED SOFA_BUILD_TESTS" OFF)
if(SOFA_VTK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
