#include <SOFAVTK/PolyDataVTKLoader.h>
#include <sofa/core/ObjectFactory.h>

#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkOFFReader.h>
#include <vtkPTSReader.h>

namespace
{

template<class Reader>
vtkSmartPointer<vtkPolyData> getPolyData(const sofa::core::objectmodel::DataFileName& fileName)
{
    vtkNew<Reader> reader;
    reader->SetFileName(fileName.getFullPath().c_str());
    reader->Update();
    return reader->GetOutput();
}

}

namespace sofavtk
{

void registerPolyDataVTKLoader(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Mesh loader")
        .add< PolyDataVTKLoader >());
}

vtkSmartPointer<vtkDataSet> PolyDataVTKLoader::getDataSet(
    const sofa::core::objectmodel::DataFileName& fileName)
{
    const std::string extension = sofa::helper::downcaseString(fileName.getExtension());

    static const std::unordered_map<std::string, std::function<vtkSmartPointer<vtkPolyData>()>> readers {
        {"ply", [&fileName](){return getPolyData<vtkPLYReader>(fileName);}},
        {"vtp", [&fileName](){return getPolyData<vtkXMLPolyDataReader>(fileName);}},
        {"obj", [&fileName](){return getPolyData<vtkOBJReader>(fileName);}},
        {"stl", [&fileName](){return getPolyData<vtkSTLReader>(fileName);}},
        {"vtk", [&fileName](){return getPolyData<vtkPolyDataReader>(fileName);}},
        {"g", [&fileName](){return getPolyData<vtkBYUReader>(fileName);}},
        {"off", [&fileName](){return getPolyData<vtkOFFReader>(fileName);}},
        {"pts", [&fileName](){return getPolyData<vtkPTSReader>(fileName);}}
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

}  // namespace sofavtk
