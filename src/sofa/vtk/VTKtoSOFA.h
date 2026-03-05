#pragma once

#include <sofa/vtk/config.h>
#include <sofa/core/loader/MeshLoader.h>
#include <vtkDataSet.h>
#include <vtkSmartPointer.h>

namespace sofavtk
{

void SOFA_VTK_API extractPoints(
    sofa::type::vector<sofa::type::Vec3>& positions,
    vtkSmartPointer<vtkDataSet> dataSet);

void SOFA_VTK_API extractCells(
    sofa::core::loader::MeshLoader& loader,
    vtkSmartPointer<vtkDataSet> dataSet
);

}
