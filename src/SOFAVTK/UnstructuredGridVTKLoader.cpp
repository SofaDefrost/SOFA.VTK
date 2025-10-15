#include <SOFAVTK/UnstructuredGridVTKLoader.h>
#include <sofa/core/ObjectFactory.h>

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>


namespace sofavtk
{

void registerUnstructuredGridVTKLoader(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Mesh loader for the VTK/VTU file format.")
        .add< UnstructuredGridVTKLoader >());
}

bool UnstructuredGridVTKLoader::doLoad()
{
    const auto& fileName = d_filename.getFullPath();
    msg_info() << "Loading VTK file: " << fileName ;

    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid;
    std::string extension;
    if (fileName.find_last_of(".") != std::string::npos)
    {
        extension = fileName.substr(fileName.find_last_of("."));
    }

    // Drop the case of the extension
    extension = sofa::helper::downcaseString(extension);

    if (extension == ".vtu")
    {
        vtkNew<vtkXMLUnstructuredGridReader> reader;
        reader->SetFileName(fileName.c_str());
        reader->Update();
        unstructuredGrid = reader->GetOutput();
    }
    else if (extension == ".vtk")
    {
        vtkNew<vtkUnstructuredGridReader> reader;
        reader->SetFileName(fileName.c_str());
        reader->Update();
        unstructuredGrid = reader->GetOutput();
    }
    else
    {
        msg_error() << "Unsupported file extension: " << extension << " for file: " << fileName << ". Supported extensions: .vtu, .vtk" ;
        return false;
    }

    return false;
}

void UnstructuredGridVTKLoader::doClearBuffers()
{

}



} // namespace sofavtk
