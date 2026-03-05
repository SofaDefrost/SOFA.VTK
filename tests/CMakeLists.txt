cmake_minimum_required(VERSION 3.12)
project(SOFAVTK_test VERSION 1.0)

find_package(Sofa.Testing REQUIRED)
find_package(VTK COMPONENTS CommonCore CommonDataModel IOXML REQUIRED)

set(SOURCE_FILES
    test.cpp
    TestCellPointData.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} Sofa.Testing SOFA.VTK ${VTK_LIBRARIES})

# VTK reader/writer implementations are loaded via factory objects that need
# to be registered before use; autoinit handles this for the test binary.
vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES ${VTK_LIBRARIES}
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
