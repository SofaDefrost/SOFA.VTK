#include <sofa/vtk/BaseVTKLoader.h>
#include <sofa/vtk/VTKtoSOFA.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkCellData.h>

namespace
{

template<class Reader>
vtkSmartPointer<vtkDataSet> getDataSet(std::string fileName)
{
    vtkNew<Reader> reader;
    reader->SetFileName(fileName.c_str());
    reader->Update();
    return reader->GetOutput();
}

}

namespace sofavtk
{

sofa::core::objectmodel::Data<BaseVTKLoader::Vec3Vector>* BaseVTKLoader::getCellVectorData(const std::string& name) const
{
    auto it = m_cellVectorData.find(name);
    if (it != m_cellVectorData.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void BaseVTKLoader::loadCellVectorData(vtkSmartPointer<vtkDataSet> dataset)
{
    // Clear any previously loaded dynamic data
    for (auto& [name, dataPtr] : m_cellVectorData)
    {
        this->removeData(dataPtr.get());
    }
    m_cellVectorData.clear();

    const auto& names = d_cellVectorDataNames.getValue();
    if (names.empty())
    {
        return;
    }

    vtkCellData* cellData = dataset->GetCellData();
    if (!cellData)
    {
        msg_warning() << "No cell data available in the dataset";
        return;
    }

    for (const auto& arrayName : names)
    {
        vtkDataArray* array = cellData->GetArray(arrayName.c_str());
        if (!array)
        {
            msg_warning() << "Cell data array '" << arrayName << "' not found in VTK file";
            continue;
        }

        if (array->GetNumberOfComponents() != 3)
        {
            msg_warning() << "Cell data array '" << arrayName << "' has "
                          << array->GetNumberOfComponents() << " components, expected 3 (Vec3)";
            continue;
        }

        // Create a new Data object for this array
        auto dataPtr = std::make_unique<sofa::core::objectmodel::Data<Vec3Vector>>();
        dataPtr->setName(arrayName);
        dataPtr->setHelp("Cell vector data loaded from VTK file");

        // Add it to the component so it becomes visible in SOFA
        this->addData(dataPtr.get(), arrayName);

        // Load the data from VTK
        auto accessor = sofa::helper::getWriteOnlyAccessor(*dataPtr);
        sofavtk::loadVTKCellData_3D(dataset, arrayName.c_str(), accessor.wref());

        msg_info() << "Loaded cell vector data '" << arrayName << "' with " << accessor->size() << " entries";

        // Store the pointer
        m_cellVectorData[arrayName] = std::move(dataPtr);
    }
}

bool BaseVTKLoader::doLoad()
{
    const auto& fileName = d_filename.getFullPath();
    msg_info() << "Loading VTK file: " << fileName ;

    vtkSmartPointer<vtkDataSet> dataSet = getDataSet(d_filename);

    if (dataSet != nullptr)
    {
        //sofavtk::listDataArrays(dataSet);

        {
            auto positions = sofa::helper::getWriteOnlyAccessor(this->d_positions);
            sofavtk::extractPoints(positions.wref(), dataSet);
        }

        sofavtk::extractCells(*this, dataSet);

        // Load user-specified cell vector data
        loadCellVectorData(dataSet);

        loadVTKData(dataSet);

        return true;
    }

    return false;
}

void BaseVTKLoader::doClearBuffers()
{
    // Clear dynamically created Data objects
    for (auto& [name, dataPtr] : m_cellVectorData)
    {
        this->removeData(dataPtr.get());
    }
    m_cellVectorData.clear();
}


}  // namespace sofavtk


namespace
{

void extractFromPolyData(
    sofavtk::BaseVTKLoader& loader,
    vtkSmartPointer<vtkPolyData> polyData)
{
    {
        auto positions = sofa::helper::getWriteOnlyAccessor(loader.d_positions);
        sofavtk::extractPoints(positions.wref(), polyData);
    }

    sofavtk::extractCells(loader, polyData);
}

}
