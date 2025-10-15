#include <sofa/vtk/UnstructuredGridVTKLoader.h>
#include <sofa/core/ObjectFactory.h>

#include <sofa/vtk/VTKtoSOFA.h>
#include <sofa/vtk/CellTypeName.h>

#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>

namespace
{

template<class Reader>
vtkSmartPointer<vtkUnstructuredGrid> getUnstructuredGrid(const sofa::core::objectmodel::DataFileName& fileName)
{
    vtkNew<Reader> reader;
    reader->SetFileName(fileName.getFullPath().c_str());
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

vtkSmartPointer<vtkDataSet> UnstructuredGridVTKLoader::getDataSet(
    const sofa::core::objectmodel::DataFileName& fileName)
{
    const std::string extension = sofa::helper::downcaseString(fileName.getExtension());

    static const std::unordered_map<std::string, std::function<vtkSmartPointer<vtkUnstructuredGrid>()>> readers {
        {"vtu", [&fileName](){return getUnstructuredGrid<vtkXMLUnstructuredGridReader>(fileName);}},
        {"vtk", [&fileName](){return getUnstructuredGrid<vtkUnstructuredGridReader>(fileName);}}
    };

    if (auto it = readers.find(extension); it != readers.end())
    {
        return it->second();
    }

    const auto supportedExtensions =
        sofa::helper::join(readers.begin(), readers.end(), [](auto& pair){return pair.first;}, ',');

    msg_error() << "Unsupported file extension: " << extension << " for file: " << fileName
                << ". Supported extensions: " << supportedExtensions;
    return nullptr;
}

} // namespace sofavtk
