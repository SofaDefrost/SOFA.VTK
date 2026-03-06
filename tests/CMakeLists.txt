cmake_minimum_required(VERSION 3.12)
project(SOFAVTK_test VERSION 1.0)

find_package(Sofa.Testing REQUIRED)
# BaseVTKLoader.h exposes vtkDataSet/vtkSmartPointer in its public interface,
# so the test needs VTK headers even though it does not use VTK directly.
find_package(VTK COMPONENTS CommonCore CommonDataModel REQUIRED)

set(SOURCE_FILES
    test.cpp
    TestCellPointData.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} Sofa.Testing SOFA.VTK ${VTK_LIBRARIES})

vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES ${VTK_LIBRARIES}
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
