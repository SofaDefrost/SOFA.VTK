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

# On Windows the test binary needs VTK and SOFA DLLs on PATH at runtime.
if(WIN32)
    set_tests_properties(${PROJECT_NAME} PROPERTIES
        ENVIRONMENT "PATH=$<TARGET_FILE_DIR:VTK::CommonCore>\;$<TARGET_FILE_DIR:Sofa.Core>\;$ENV{PATH}")
endif()
