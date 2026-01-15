#pragma once

#include <sofa/vtk/config.h>
#include <sofa/core/loader/MeshLoader.h>
#include <vtkDataSet.h>
#include <vtkSmartPointer.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>

namespace sofavtk
{

void SOFA_VTK_API extractPoints(
    sofa::type::vector<sofa::type::Vec3>& positions,
    vtkSmartPointer<vtkDataSet> dataSet);

void SOFA_VTK_API extractCells(
    sofa::core::loader::MeshLoader& loader,
    vtkSmartPointer<vtkDataSet> dataSet
);

template<typename DataType, int NumComponents>
void extractCellData(vtkSmartPointer<vtkDataSet> dataSet, const char* arrayName,
                     sofa::type::vector<DataType>& data)
{
    vtkCellData* cellData = dataSet->GetCellData();
    if (!cellData) return;

    vtkDataArray* array = cellData->GetArray(arrayName);
    if (!array) return;

    const auto nbCells = dataSet->GetNumberOfCells();
    data.resize(nbCells);

    for (vtkIdType cellId = 0; cellId < nbCells; ++cellId)
    {
        const double* values = array->GetTuple(cellId);

        if constexpr (NumComponents == 1)
            data[cellId] = static_cast<DataType>(values[0]);
        else
            data[cellId] = DataType(values);
    }
}

}
