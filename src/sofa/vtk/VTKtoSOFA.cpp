#include <sofa/vtk/CellTypeName.h>
#include <sofa/vtk/VTKtoSOFA.h>
#include <vtkCell.h>
#include <vtkCellType.h>
#include <vtkCellTypes.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

namespace sofavtk
{

struct BaseCellData
{
    virtual void fill(vtkSmartPointer<vtkDataSet> dataSet) = 0;
};

template<VTKCellType cellType, class T>
struct CellData : BaseCellData
{
    explicit CellData(sofa::Data<T>* data) : m_data(data) {}

    void fill(vtkSmartPointer<vtkDataSet> dataSet) override
    {
        auto accessor = sofa::helper::getWriteOnlyAccessor(*m_data);

        const auto nbCells = dataSet->GetNumberOfCells();
        for (vtkIdType i = 0; i < nbCells; ++i)
        {
            vtkCell* cell = dataSet->GetCell(i);
            if (cell->GetCellType() == cellType)
            {
                auto& element = accessor.emplace_back();
                for (int j = 0; j < cell->GetNumberOfPoints(); ++j)
                {
                    element[j] = static_cast<sofa::Index>(cell->GetPointId(j));
                }
            }
        }
    }
private:
    sofa::Data<T>* m_data;
};

template<VTKCellType cellType, class Container>
void registerInMap(std::unordered_map<VTKCellType, std::unique_ptr<BaseCellData>>& map, Container* container)
{
    using ContainerT = std::remove_reference_t<decltype(*container)>;
    map.emplace(cellType, std::make_unique<CellData<cellType, typename ContainerT::value_type>>(container));
}

std::unordered_map<VTKCellType, std::unique_ptr<BaseCellData>> makeCellDataMap(sofa::core::loader::MeshLoader& loader)
{
    std::unordered_map<VTKCellType, std::unique_ptr<BaseCellData>> dataMap;

    registerInMap<VTK_POLY_LINE>(dataMap, &loader.d_polylines);

    registerInMap<VTK_LINE>(dataMap, &loader.d_edges);
    registerInMap<VTK_TRIANGLE>(dataMap, &loader.d_triangles);
    registerInMap<VTK_QUAD>(dataMap, &loader.d_quads);
    registerInMap<VTK_POLYGON>(dataMap, &loader.d_polygons);

    registerInMap<VTK_TETRA>(dataMap, &loader.d_tetrahedra);
    registerInMap<VTK_HEXAHEDRON>(dataMap, &loader.d_hexahedra);
    registerInMap<VTK_WEDGE>(dataMap, &loader.d_pentahedra);
    registerInMap<VTK_PYRAMID>(dataMap, &loader.d_pyramids);

    return dataMap;
}

}


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

void extractCells(sofa::core::loader::MeshLoader& loader, vtkSmartPointer<vtkDataSet> dataSet)
{
    auto dataMap = makeCellDataMap(loader);

    vtkSmartPointer <vtkCellTypes> types = vtkSmartPointer <vtkCellTypes>::New();
    dataSet->GetCellTypes(types);
    const vtkIdType nbElementTypes = types->GetNumberOfTypes();
    for (vtkIdType i = 0; i < nbElementTypes; ++i)
    {
        const auto type = types->GetCellType(i);
        if (auto it = dataMap.find(static_cast<VTKCellType>(type)); it != dataMap.end())
        {
            it->second->fill(dataSet);
        }
        else
        {
            msg_error(&loader) << "Unsupported cell type: "
                << sofavtk::getCellTypeName(static_cast<VTKCellType>(type)) << " (" << static_cast<VTKCellType>(type) << ")";
        }
    }
}


void listDataArrays(vtkSmartPointer<vtkDataSet> dataset)
{
    if (!dataset) return;

    // --- Point Data ---
    vtkPointData* pointData = dataset->GetPointData();
    std::cout << "Point Data Arrays:" << std::endl;
    for (int i = 0; i < pointData->GetNumberOfArrays(); ++i)
    {
        vtkDataArray* array = pointData->GetArray(i);
        std::cout << "  [" << i << "] " << array->GetName()
                  << "  components: " << array->GetNumberOfComponents()
                  << "  tuples: " << array->GetNumberOfTuples() << std::endl;
    }

    // --- Cell Data ---
    vtkCellData* cellData = dataset->GetCellData();
    std::cout << "Cell Data Arrays:" << std::endl;
    for (int i = 0; i < cellData->GetNumberOfArrays(); ++i)
    {
        vtkDataArray* array = cellData->GetArray(i);
        std::cout << "  [" << i << "] " << array->GetName()
                  << "  components: " << array->GetNumberOfComponents()
                  << "  tuples: " << array->GetNumberOfTuples() << std::endl;
    }
}


void loadVTKCellData_3D(vtkSmartPointer<vtkDataSet> dataSet, const char* cellDataName, sofa::type::vector<sofa::type::Vec3>& data)
{
    vtkCellData* cellData = dataSet->GetCellData();
    
    vtkDataArray* array = cellData->GetArray(cellDataName);

    if (array)
    {
        const auto nbCells = dataSet->GetNumberOfCells();
        data.resize(nbCells);

        std::cout << "vtkCellData found, loading: " << cellDataName << " -> " << nbCells << " cells" << std::endl;
        for (vtkIdType cellId = 0; cellId < nbCells; ++cellId)
        {
            double value[3];  // assuming max 3 components
            array->GetTuple(cellId, value);
            //std::cout << "Cell " << cellId << ": ";

            data[cellId] = sofa::type::Vec3(value[0], value[1], value[2]);
        }
    }
}   

}  // namespace sofavtk
