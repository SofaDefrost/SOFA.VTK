#include <SOFAVTK/UnstructuredGridVTKLoader.h>
#include <sofa/core/ObjectFactory.h>

#include <SOFAVTK/VTKtoSOFA.h>
#include <SOFAVTK/CellTypeName.h>

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
        unstructuredGrid = getUnstructuredGrid<vtkXMLUnstructuredGridReader>(fileName.c_str());
    }
    else if (extension == "vtk")
    {
        unstructuredGrid = getUnstructuredGrid<vtkUnstructuredGridReader>(fileName.c_str());
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

void extractFromUnstructuredGrid(
    sofavtk::UnstructuredGridVTKLoader& loader,
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid)
{
    {
        auto positions = sofa::helper::getWriteOnlyAccessor(loader.d_positions);
        sofavtk::extractPoints(positions.wref(), unstructuredGrid);
    }

    sofavtk::extractCells(loader, unstructuredGrid);

}
}
