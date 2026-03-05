#pragma once

#include <sofa/vtk/config.h>
#include <sofa/core/loader/MeshLoader.h>
#include <vtkDataSet.h>
#include <vtkSmartPointer.h>
#include <vtkArrayDispatch.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataArrayRange.h>

namespace sofavtk
{

void SOFA_VTK_API extractPoints(
    sofa::type::vector<sofa::type::Vec3>& positions,
    vtkSmartPointer<vtkDataSet> dataSet);

void SOFA_VTK_API extractCells(
    sofa::core::loader::MeshLoader& loader,
    vtkSmartPointer<vtkDataSet> dataSet
);

namespace detail
{

/// Worker functor invoked by vtkArrayDispatch with the concrete array type.
/// Falls back to the vtkDataArray virtual API when dispatch finds no match.
template<typename SofaType, int NumComponents>
struct CellDataWorker
{
    sofa::type::vector<SofaType>& data;

    template<typename ArrayT>
    void operator()(ArrayT* array) const
    {
        const vtkIdType nbCells = array->GetNumberOfTuples();
        data.resize(nbCells);

        if constexpr (NumComponents == 1)
        {
            vtkIdType i = 0;
            for (const auto v : vtk::DataArrayValueRange<1>(array))
                data[i++] = static_cast<SofaType>(v);
        }
        else
        {
            vtkIdType i = 0;
            for (const auto tuple : vtk::DataArrayTupleRange<NumComponents>(array))
            {
                for (int c = 0; c < NumComponents; ++c)
                    data[i][c] = static_cast<typename SofaType::value_type>(tuple[c]);
                ++i;
            }
        }
    }
};

} // namespace detail

template<typename SofaType, int NumComponents>
void extractCellData(vtkSmartPointer<vtkDataSet> dataSet, const char* arrayName,
                     sofa::type::vector<SofaType>& data)
{
    vtkCellData* cellData = dataSet->GetCellData();
    if (!cellData) return;

    vtkDataArray* array = cellData->GetArray(arrayName);
    if (!array) return;

    detail::CellDataWorker<SofaType, NumComponents> worker{data};
    using Dispatcher = vtkArrayDispatch::DispatchByValueType<vtkArrayDispatch::AllTypes>;
    if (!Dispatcher::Execute(array, worker))
        worker(array);  // fallback via vtkDataArray virtual API
}

}
