#include <SOFAVTK/UnstructuredGridVTKLoader.h>
#include <sofa/core/ObjectFactory.h>

#include <SOFAVTK/VTKtoSOFA.h>

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>

namespace
{
void extractFromUnstructuredGrid(
    sofavtk::UnstructuredGridVTKLoader& loader,
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid);

template<class Reader>
vtkSmartPointer<vtkUnstructuredGrid> getUnstructuredGrid(std::string fileName)
{
    vtkNew<Reader> reader;
    reader->SetFileName(fileName.c_str());
    reader->Update();
    return reader->GetOutput();
}
}

namespace sofavtk
{

void registerUnstructuredGridVTKLoader(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Mesh loader for unstructured grids")
        .add< UnstructuredGridVTKLoader >());
}

bool UnstructuredGridVTKLoader::doLoad()
{
    const auto& fileName = d_filename.getFullPath();
    msg_info() << "Loading VTK file: " << fileName ;

    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid;
    const std::string extension = sofa::helper::downcaseString(d_filename.getExtension());

    if (extension == "vtu")
    {
        getUnstructuredGrid<vtkXMLUnstructuredGridReader>(fileName.c_str());
    }
    else if (extension == "vtk")
    {
        getUnstructuredGrid<vtkUnstructuredGridReader>(fileName.c_str());
    }
    else
    {
        msg_error() << "Unsupported file extension: " << extension << " for file: " << fileName << ". Supported extensions: .vtu, .vtk" ;
        return false;
    }

    if (unstructuredGrid != nullptr)
    {
        extractFromUnstructuredGrid(*this, unstructuredGrid);
        return true;
    }

    return false;
}

void UnstructuredGridVTKLoader::doClearBuffers()
{

}

} // namespace sofavtk

namespace
{

struct BaseCellData
{
    virtual void fill(vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid) = 0;
};

template<VTKCellType cellType, class T>
struct CellData : BaseCellData
{
    explicit CellData(sofa::Data<T>* data) : m_data(data) {}

    void fill(vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid) override
    {
        auto accessor = sofa::helper::getWriteOnlyAccessor(*m_data);

        auto cells = vtkSmartPointer <vtkIdTypeArray>::New();
        unstructuredGrid->GetIdsOfCellsOfType(cellType, cells);

        const auto nbElements = cells->GetDataSize();
        accessor.resize(nbElements);

        for (vtkIdType i = 0; i < nbElements; ++i)
        {
            vtkIdType cell_id = cells->GetValue(i);
            vtkCell* cell = unstructuredGrid->GetCell(cell_id);

            for (int j = 0; j < cell->GetNumberOfPoints(); ++j)
            {
                accessor[i][j] = static_cast<sofa::Index>(cell->GetPointId(j));
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

std::string getCellTypeName(VTKCellType cellType)
{
#define CELL_TYPE(type) std::make_pair(type, std::string(#type))

    static std::unordered_map cellTypeNames{
        CELL_TYPE(VTK_EMPTY_CELL),
        CELL_TYPE(VTK_VERTEX),
        CELL_TYPE(VTK_POLY_VERTEX),
        CELL_TYPE(VTK_LINE),
        CELL_TYPE(VTK_POLY_LINE),
        CELL_TYPE(VTK_TRIANGLE),
        CELL_TYPE(VTK_TRIANGLE_STRIP),
        CELL_TYPE(VTK_POLYGON),
        CELL_TYPE(VTK_PIXEL),
        CELL_TYPE(VTK_QUAD),
        CELL_TYPE(VTK_TETRA),
        CELL_TYPE(VTK_VOXEL),
        CELL_TYPE(VTK_HEXAHEDRON),
        CELL_TYPE(VTK_WEDGE),
        CELL_TYPE(VTK_PYRAMID),
        CELL_TYPE(VTK_PENTAGONAL_PRISM),
        CELL_TYPE(VTK_HEXAGONAL_PRISM),
        CELL_TYPE(VTK_QUADRATIC_EDGE),
        CELL_TYPE(VTK_QUADRATIC_TRIANGLE),
        CELL_TYPE(VTK_QUADRATIC_QUAD),
        CELL_TYPE(VTK_QUADRATIC_POLYGON),
        CELL_TYPE(VTK_QUADRATIC_TETRA),
        CELL_TYPE(VTK_QUADRATIC_HEXAHEDRON),
        CELL_TYPE(VTK_QUADRATIC_WEDGE),
        CELL_TYPE(VTK_QUADRATIC_PYRAMID),
        CELL_TYPE(VTK_BIQUADRATIC_QUAD),
        CELL_TYPE(VTK_TRIQUADRATIC_HEXAHEDRON),
        CELL_TYPE(VTK_TRIQUADRATIC_PYRAMID),
        CELL_TYPE(VTK_QUADRATIC_LINEAR_QUAD),
        CELL_TYPE(VTK_QUADRATIC_LINEAR_WEDGE),
        CELL_TYPE(VTK_BIQUADRATIC_QUADRATIC_WEDGE),
        CELL_TYPE(VTK_BIQUADRATIC_QUADRATIC_HEXAHEDRON),
        CELL_TYPE(VTK_BIQUADRATIC_TRIANGLE),
        CELL_TYPE(VTK_CUBIC_LINE),
        CELL_TYPE(VTK_CONVEX_POINT_SET),
        CELL_TYPE(VTK_POLYHEDRON),
        CELL_TYPE(VTK_PARAMETRIC_CURVE),
        CELL_TYPE(VTK_PARAMETRIC_SURFACE),
        CELL_TYPE(VTK_PARAMETRIC_TRI_SURFACE),
        CELL_TYPE(VTK_PARAMETRIC_QUAD_SURFACE),
        CELL_TYPE(VTK_PARAMETRIC_TETRA_REGION),
        CELL_TYPE(VTK_PARAMETRIC_HEX_REGION),
        CELL_TYPE(VTK_HIGHER_ORDER_EDGE),
        CELL_TYPE(VTK_HIGHER_ORDER_TRIANGLE),
        CELL_TYPE(VTK_HIGHER_ORDER_QUAD),
        CELL_TYPE(VTK_HIGHER_ORDER_POLYGON),
        CELL_TYPE(VTK_HIGHER_ORDER_TETRAHEDRON),
        CELL_TYPE(VTK_HIGHER_ORDER_WEDGE),
        CELL_TYPE(VTK_HIGHER_ORDER_PYRAMID),
        CELL_TYPE(VTK_HIGHER_ORDER_HEXAHEDRON),
        CELL_TYPE(VTK_LAGRANGE_CURVE),
        CELL_TYPE(VTK_LAGRANGE_TRIANGLE),
        CELL_TYPE(VTK_LAGRANGE_QUADRILATERAL),
        CELL_TYPE(VTK_LAGRANGE_TETRAHEDRON),
        CELL_TYPE(VTK_LAGRANGE_HEXAHEDRON),
        CELL_TYPE(VTK_LAGRANGE_WEDGE),
        CELL_TYPE(VTK_LAGRANGE_PYRAMID),
        CELL_TYPE(VTK_BEZIER_CURVE),
        CELL_TYPE(VTK_BEZIER_TRIANGLE),
        CELL_TYPE(VTK_BEZIER_QUADRILATERAL),
        CELL_TYPE(VTK_BEZIER_TETRAHEDRON),
        CELL_TYPE(VTK_BEZIER_HEXAHEDRON),
        CELL_TYPE(VTK_BEZIER_WEDGE),
        CELL_TYPE(VTK_BEZIER_PYRAMID)
    };
#undef CELL_TYPE

    auto it = cellTypeNames.find(cellType);
    if (it != cellTypeNames.end())
    {
        return it->second;
    }
    return "Unknown";
}

void extractFromUnstructuredGrid(
    sofavtk::UnstructuredGridVTKLoader& loader,
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid)
{
    auto positions = sofa::helper::getWriteOnlyAccessor(loader.d_positions);
    sofavtk::extractPoints(positions.wref(), unstructuredGrid);

    auto dataMap = makeCellDataMap(loader);

    vtkSmartPointer <vtkCellTypes> types = vtkSmartPointer <vtkCellTypes>::New();
    unstructuredGrid->GetCellTypes(types);
    const vtkIdType nbElementTypes = types->GetNumberOfTypes();
    for (vtkIdType i = 0; i < nbElementTypes; ++i)
    {
        const auto type = types->GetCellType(i);
        if (auto it = dataMap.find(static_cast<VTKCellType>(type)); it != dataMap.end())
        {
            it->second->fill(unstructuredGrid);
        }
        else
        {
            msg_error(&loader) << "Unsupported cell type: "
                << getCellTypeName(static_cast<VTKCellType>(type)) << " (" << static_cast<VTKCellType>(type) << ")";
        }
    }
}
}
