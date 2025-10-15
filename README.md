# SOFA.VTK

## Description
SOFA plugin using VTK to read meshes

## Dependencies

This library relies on VTK (Visualization Toolkit). You need to make sure CMake can locate your VTK installation when building your project.

1. Install VTK from your package manager or build it from source.
   Once installed, note the directory containing vkt-config.cmake, for example:

```
/usr/lib/cmake/vtk-9.5/
C:/Program Files/VTK/lib/cmake/vtk-9.5/
<your_build_dir>/VTK-build/
```

2. If CMake cannot find VTK automatically, specify its location manually:

Using VTK_DIR:
```
cmake -DVTK_DIR=/path/to/VTK/lib/cmake/vtk-9.5 ..
```

Or using CMAKE_PREFIX_PATH:
```
cmake -DCMAKE_PREFIX_PATH="/path/to/VTK/lib/cmake/vtk-9.5" ..
```

Both methods point CMake to the folder containing vkt-config.cmake

## Examples

```xml
<PolyDataVTKLoader name="VtkLoader" filename="mesh/liver.vtk" flipNormals="0" printLog="true" createSubelements="true"/>
<OglModel name="VisualModel" src="@VtkLoader" color="red"/>
```

```xml
<UnstructuredGridVTKLoader name="VtkLoader" filename="mesh/seaMonster2-volume1.vtu" flipNormals="0" printLog="true" createSubelements="true"/>
<OglModel name="VisualModel" src="@VtkLoader" color="red"/>
```
