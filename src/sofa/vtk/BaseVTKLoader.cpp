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

/// Dispatch worker for scalar cell data arrays.
/// The concrete array type ArrayT is deduced by vtkArrayDispatch, so T = vtk::GetAPIType<ArrayT>
/// is the exact native C++ scalar type. Data is copied losslessly with no manual type checks.
struct ScalarCellDataWorker
{
    sofavtk::BaseVTKLoader& loader;
    const std::string& arrayName;
    std::unique_ptr<sofa::core::objectmodel::BaseData> result;
    vtkIdType numLoaded = 0;

    template<typename ArrayT>
    void operator()(ArrayT* array)
    {
        using T = vtk::GetAPIType<ArrayT>;

        auto dataPtr = std::make_unique<sofa::core::objectmodel::Data<sofa::type::vector<T>>>();
        dataPtr->setName(arrayName);
        dataPtr->setHelp("Cell data loaded from VTK file");

        {
            auto accessor = sofa::helper::getWriteOnlyAccessor(*dataPtr);
            auto& vec = accessor.wref();
            vec.resize(array->GetNumberOfTuples());
            vtkIdType i = 0;
            for (const auto v : vtk::DataArrayValueRange<1>(array))
                vec[i++] = v;
            numLoaded = i;
        }

        loader.addData(dataPtr.get(), arrayName);
        result = std::move(dataPtr);
    }
};

}

namespace sofavtk
{

void BaseVTKLoader::loadCellDataArrayByName(vtkSmartPointer<vtkDataSet> dataset,
                                            const std::string& arrayName)
{
    vtkCellData* cellData = dataset->GetCellData();
    if (!cellData)
    {
        msg_warning() << "No cell data available in the dataset";
        return;
    }

    vtkDataArray* array = cellData->GetArray(arrayName.c_str());
    if (!array)
    {
        msg_warning() << "Cell data array '" << arrayName << "' not found in VTK file";
        return;
    }

    const int numComponents = array->GetNumberOfComponents();

    if (numComponents == 1)
    {
        ScalarCellDataWorker worker{*this, arrayName};
        using Dispatcher = vtkArrayDispatch::DispatchByValueType<vtkArrayDispatch::AllTypes>;
        if (!Dispatcher::Execute(array, worker))
            worker(array);  // fallback: ArrayT = vtkDataArray, T = double
        if (worker.result)
        {
            msg_info() << "Loaded cell data '" << arrayName << "' with " << worker.numLoaded << " entries";
            m_cellData[arrayName] = std::move(worker.result);
        }
        return;
    }

    // Multi-component arrays: always mapped to Vec<N, SReal>
    switch (numComponents)
    {
    case 2:
        loadCellDataArray<sofa::type::Vec<2, SReal>, 2>(dataset, arrayName);
        break;
    case 3:
        loadCellDataArray<sofa::type::Vec<3, SReal>, 3>(dataset, arrayName);
        break;
    case 4:
        loadCellDataArray<sofa::type::Vec<4, SReal>, 4>(dataset, arrayName);
        break;
    case 6:
        loadCellDataArray<sofa::type::Vec<6, SReal>, 6>(dataset, arrayName);
        break;
    case 9:
        loadCellDataArray<sofa::type::Vec<9, SReal>, 9>(dataset, arrayName);
        break;
    default:
        msg_warning() << "Cell data array '" << arrayName
            << "' has unsupported number of components: " << numComponents;
        break;
    }
}

template<typename DataType, int NumComponents>
void BaseVTKLoader::loadCellDataArray(vtkSmartPointer<vtkDataSet> dataset,
                                      const std::string& arrayName)
{
    auto dataPtr = std::make_unique<sofa::core::objectmodel::Data<sofa::type::vector<DataType>>>();
    dataPtr->setName(arrayName);
    dataPtr->setHelp("Cell data loaded from VTK file");
    this->addData(dataPtr.get(), arrayName);

    auto accessor = sofa::helper::getWriteOnlyAccessor(*dataPtr);
    sofavtk::extractCellData<DataType, NumComponents>(dataset, arrayName.c_str(), accessor.wref());

    msg_info() << "Loaded cell data '" << arrayName << "' with " << accessor->size() << " entries";

    m_cellData[arrayName] = std::move(dataPtr);
}

bool BaseVTKLoader::doLoad()
{
    const auto& fileName = d_filename.getFullPath();
    msg_info() << "Loading VTK file: " << fileName ;

    vtkSmartPointer<vtkDataSet> dataSet = getDataSet(d_filename);

    if (dataSet != nullptr)
    {
        {
            auto positions = sofa::helper::getWriteOnlyAccessor(this->d_positions);
            sofavtk::extractPoints(positions.wref(), dataSet);
        }

        sofavtk::extractCells(*this, dataSet);

        for (const auto& arrayName : d_cellDataNames.getValue())
            loadCellDataArrayByName(dataSet, arrayName);

        return true;
    }

    return false;
}

void BaseVTKLoader::doClearBuffers()
{
    for (auto& pair : m_cellData)
        this->removeData(pair.second.get());

    m_cellData.clear();
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
