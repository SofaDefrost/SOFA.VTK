#include <SOFAVTK/VTKtoSOFA.h>

#include <vtkPoints.h>

namespace sofavtk
{

void extractPoints(
    sofa::type::vector<sofa::type::Vec3>& positions,
    vtkSmartPointer<vtkDataSet> dataSet)
{
    assert(dataSet);

    static constexpr sofa::Index dimension = 3;

    auto* points = dataSet->GetPoints();
    const auto nbPoints = points->GetNumberOfPoints();

    positions.resize(nbPoints);

    for (vtkIdType i = 0; i < nbPoints; ++i)
    {
        double* point = points->GetPoint(i);
        for (sofa::Index j = 0; j < dimension; ++j)
        {
            positions[i][j] = point[j];
        }
    }
}

}
