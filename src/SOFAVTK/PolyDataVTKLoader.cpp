#include <SOFAVTK/PolyDataVTKLoader.h>
#include <sofa/core/ObjectFactory.h>

#include <vtkSmartPointer.h>

#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>

#include <SOFAVTK/VTKtoSOFA.h>

namespace
{

template<class Reader>
vtkSmartPointer<vtkPolyData> getPolyData(std::string fileName)
{
    vtkNew<Reader> reader;
    reader->SetFileName(fileName.c_str());
    reader->Update();
    return reader->GetOutput();
}

void extractFromPolyData(
    sofavtk::PolyDataVTKLoader& loader,
    vtkSmartPointer<vtkPolyData> polyData);

}

namespace sofavtk
{

void registerPolyDataVTKLoader(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Mesh loader")
        .add< PolyDataVTKLoader >());
}

bool PolyDataVTKLoader::doLoad()
{
    const auto& fileName = d_filename.getFullPath();
    msg_info() << "Loading VTK file: " << fileName ;

    vtkSmartPointer<vtkPolyData> polyData;
    const std::string extension = sofa::helper::downcaseString(d_filename.getExtension());

    if (extension == "ply")
    {
        polyData = getPolyData<vtkPLYReader>(fileName);
    }
    else if (extension == "vtp")
    {
        polyData = getPolyData<vtkXMLPolyDataReader>(fileName);
    }
    else if (extension == "obj")
    {
        polyData = getPolyData<vtkOBJReader>(fileName);
    }
    else if (extension == "stl")
    {
        polyData = getPolyData<vtkSTLReader>(fileName);
    }
    else if (extension == "vtk")
    {
        polyData = getPolyData<vtkPolyDataReader>(fileName);
    }
    else if (extension == "g")
    {
        polyData = getPolyData<vtkBYUReader>(fileName);
    }
    else
    {
        msg_error() << "Unsupported file extension: " << extension << " for file: " << fileName << ". Supported extensions: ..." ;
        return false;
    }

    if (polyData != nullptr)
    {
        extractFromPolyData(*this, polyData);
        return true;
    }

    return false;
}

void PolyDataVTKLoader::doClearBuffers() {}

}  // namespace sofavtk

namespace
{

void extractFromPolyData(
    sofavtk::PolyDataVTKLoader& loader,
    vtkSmartPointer<vtkPolyData> polyData)
{
    {
        auto positions = sofa::helper::getWriteOnlyAccessor(loader.d_positions);
        sofavtk::extractPoints(positions.wref(), polyData);
    }

    sofavtk::extractCells(loader, polyData);
}

}
