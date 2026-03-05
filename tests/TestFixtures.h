#pragma once

#include <string>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkTypeFloat32Array.h>
#include <vtkTypeFloat64Array.h>
#include <vtkTypeInt8Array.h>
#include <vtkTypeInt32Array.h>
#include <vtkTypeInt64Array.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridWriter.h>

// Writes a 5-point / 4-tet unstructured grid (.vtu) with known cell and point
// data arrays so tests can load it and assert exact values.
//
// Geometry:
//   Points : (0,0,0) (1,0,0) (0,1,0) (0,0,1) (1,1,1)
//   Tets   : {0,1,2,4} {0,1,3,4} {0,2,3,4} {1,2,3,4}
//
// Cell data  (4 entries per array):
//   "pressure"          float    scalar  {1, 2, 3, 4}
//   "material_id"       int32    scalar  {10, 20, 30, 40}
//   "fiber_direction"   double   vec3    {(1,0,0),(0,1,0),(0,0,1),(0.577,...)}
//   "int8_data"         int8     scalar  {1, 2, 3, 4}
//   "int32_data"        int32    scalar  {100, 200, 300, 400}
//   "int64_data"        int64    scalar  {10000, 20000, 30000, 40000}
//   "too_many"          double   10-comp (all zeros)
//
// Point data (5 entries per array):
//   "temperature"       double   scalar  {0.1, 0.2, 0.3, 0.4, 0.5}
//   "velocity"          double   vec3    {(1,0,0),(0,1,0),(0,0,1),(2,0,0),(0,2,0)}

inline void writeTestFixture(const std::string& path)
{
    vtkNew<vtkPoints> pts;
    pts->InsertNextPoint(0, 0, 0);
    pts->InsertNextPoint(1, 0, 0);
    pts->InsertNextPoint(0, 1, 0);
    pts->InsertNextPoint(0, 0, 1);
    pts->InsertNextPoint(1, 1, 1);

    vtkNew<vtkUnstructuredGrid> grid;
    grid->SetPoints(pts);

    vtkIdType t0[4] = {0, 1, 2, 4};
    vtkIdType t1[4] = {0, 1, 3, 4};
    vtkIdType t2[4] = {0, 2, 3, 4};
    vtkIdType t3[4] = {1, 2, 3, 4};
    grid->InsertNextCell(VTK_TETRA, 4, t0);
    grid->InsertNextCell(VTK_TETRA, 4, t1);
    grid->InsertNextCell(VTK_TETRA, 4, t2);
    grid->InsertNextCell(VTK_TETRA, 4, t3);

    // ---- Cell data ----

    vtkNew<vtkTypeFloat32Array> pressure;
    pressure->SetName("pressure");
    for (float v : {1.0f, 2.0f, 3.0f, 4.0f})
        pressure->InsertNextValue(v);
    grid->GetCellData()->AddArray(pressure);

    vtkNew<vtkTypeInt32Array> materialId;
    materialId->SetName("material_id");
    for (int v : {10, 20, 30, 40})
        materialId->InsertNextValue(v);
    grid->GetCellData()->AddArray(materialId);

    vtkNew<vtkTypeFloat64Array> fiberDir;
    fiberDir->SetName("fiber_direction");
    fiberDir->SetNumberOfComponents(3);
    double fd[][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0.577, 0.577, 0.577}};
    for (auto& t : fd)
        fiberDir->InsertNextTuple(t);
    grid->GetCellData()->AddArray(fiberDir);

    vtkNew<vtkTypeInt8Array> int8Data;
    int8Data->SetName("int8_data");
    for (int v : {1, 2, 3, 4})
        int8Data->InsertNextValue(static_cast<signed char>(v));
    grid->GetCellData()->AddArray(int8Data);

    vtkNew<vtkTypeInt32Array> int32Data;
    int32Data->SetName("int32_data");
    for (int v : {100, 200, 300, 400})
        int32Data->InsertNextValue(v);
    grid->GetCellData()->AddArray(int32Data);

    vtkNew<vtkTypeInt64Array> int64Data;
    int64Data->SetName("int64_data");
    for (long long v : {10000LL, 20000LL, 30000LL, 40000LL})
        int64Data->InsertNextValue(v);
    grid->GetCellData()->AddArray(int64Data);

    // 10-component array to exercise the >9-components warning path
    vtkNew<vtkTypeFloat64Array> tooMany;
    tooMany->SetName("too_many");
    tooMany->SetNumberOfComponents(10);
    double z[10] = {};
    for (int i = 0; i < 4; ++i)
        tooMany->InsertNextTuple(z);
    grid->GetCellData()->AddArray(tooMany);

    // ---- Point data ----

    vtkNew<vtkTypeFloat64Array> temperature;
    temperature->SetName("temperature");
    for (double v : {0.1, 0.2, 0.3, 0.4, 0.5})
        temperature->InsertNextValue(v);
    grid->GetPointData()->AddArray(temperature);

    vtkNew<vtkTypeFloat64Array> velocity;
    velocity->SetName("velocity");
    velocity->SetNumberOfComponents(3);
    double vt[][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {2, 0, 0}, {0, 2, 0}};
    for (auto& t : vt)
        velocity->InsertNextTuple(t);
    grid->GetPointData()->AddArray(velocity);

    vtkNew<vtkXMLUnstructuredGridWriter> writer;
    writer->SetFileName(path.c_str());
    writer->SetInputData(grid);
    writer->Write();
}
