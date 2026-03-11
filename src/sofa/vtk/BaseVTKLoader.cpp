#include <sofa/vtk/BaseVTKLoader.h>
#include <sofa/vtk/VTKtoSOFA.h>
#include <vtkArrayDispatch.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkDataArrayRange.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace
{

template<class Reader>
vtkSmartPointer<vtkDataSet> getDataSet(const std::string& fileName)
{
    vtkNew<Reader> reader;
    reader->SetFileName(fileName.c_str());
    reader->Update();
    return reader->GetOutput();
}

// Remap long and unsigned long which are platform dependent to a canonical fixed-width type
template<typename T> struct CanonicalLong      { using type = T;};
template<> struct CanonicalLong<long>          { using type = std::conditional_t<sizeof(long) == 4, int, long long>;};
template<> struct CanonicalLong<unsigned long> { using type = std::conditional_t<sizeof(unsigned long) == 4, unsigned int, unsigned long long>; };

template<typename T>
using CanonicalLong_t = typename CanonicalLong<T>::type;

struct ScalarDataWorker
{
    sofavtk::BaseVTKLoader& loader;
    const std::string& arrayName;
    std::unique_ptr<sofa::core::objectmodel::BaseData> result;

    template<typename ArrayT>
    void operator()(ArrayT* array)
    {
        using T = CanonicalLong_t<vtk::GetAPIType<ArrayT>>;

        auto dataPtr = std::make_unique<sofa::core::objectmodel::Data<sofa::type::vector<T>>>();
        dataPtr->setName(arrayName);
        dataPtr->setHelp("Data array loaded from VTK file");

        {
            auto accessor = sofa::helper::getWriteOnlyAccessor(*dataPtr);
            auto& vec = accessor.wref();
            vec.resize(array->GetNumberOfTuples());
            auto values = vtk::DataArrayValueRange<1>(array);
            const vtkIdType n = static_cast<vtkIdType>(array->GetNumberOfTuples());
            for (vtkIdType i = 0; i < n; ++i)
                vec[i] = values[i];
        }

        loader.addData(dataPtr.get(), arrayName);
        result = std::move(dataPtr);
    }
};

struct MultiComponentDataWorker
{
    sofavtk::BaseVTKLoader& loader;
    const std::string& arrayName;
    int numComponents;
    std::unique_ptr<sofa::core::objectmodel::BaseData> result;

    template<typename ArrayT>
    void operator()(ArrayT* array)
    {
        using T = CanonicalLong_t<vtk::GetAPIType<ArrayT>>;
        dispatchN<T, 2>(array);
    }

private:
    // Recursively dispatch fill function specialization based on the number of components until a 
    // match is found or the max supported is reached.
    template<typename T, int N, typename ArrayT>
    void dispatchN(ArrayT* array)
    {
        if (numComponents == N)
            fill<T, N>(array);
        else if constexpr (N < 9)
            dispatchN<T, N + 1>(array);
    }

    template<typename T, int N, typename ArrayT>
    void fill(ArrayT* array)
    {
        auto dataPtr = std::make_unique<sofa::core::objectmodel::Data<sofa::type::vector<sofa::type::Vec<N, T>>>>();
        dataPtr->setName(arrayName);
        dataPtr->setHelp("Data array loaded from VTK file");

        {
            auto accessor = sofa::helper::getWriteOnlyAccessor(*dataPtr);
            auto& vec = accessor.wref();
            vec.resize(array->GetNumberOfTuples());
            auto tuples = vtk::DataArrayTupleRange<N>(array);
            const vtkIdType n = static_cast<vtkIdType>(array->GetNumberOfTuples());
            for (vtkIdType i = 0; i < n; ++i)
            {
                for (int c = 0; c < N; ++c)
                    vec[i][c] = tuples[i][c];
            }
        }

        loader.addData(dataPtr.get(), arrayName);
        result = std::move(dataPtr);
    }
};

}

namespace sofavtk
{

void BaseVTKLoader::loadDataArrayByName(vtkFieldData* fieldData, const std::string& arrayName,
                                        std::map<std::string, std::unique_ptr<sofa::core::objectmodel::BaseData>>& storage)
{
    vtkDataArray* array = fieldData->GetArray(arrayName.c_str());
    if (!array)
    {
        msg_warning() << fieldData->GetClassName() << " array '" << arrayName << "' not found in VTK file";
        return;
    }

    const int numComponents = array->GetNumberOfComponents();

    // Explicitly supported value types. Each maps to a fixed-size C++ type through vtkArrayDispatch
    // long and unsigned long are included and remapped via CanonicalLong_t to int/long long based 
    // on their size on the current platform.
    using SupportedTypes = vtkTypeList::Create<
        float, double,
        signed char, unsigned char,
        int, unsigned int,
        long, unsigned long,
        long long, unsigned long long>;
    using Dispatcher = vtkArrayDispatch::DispatchByValueType<SupportedTypes>;

    auto dispatch = [&](auto& worker) {
        if (!Dispatcher::Execute(array, worker))
            worker(array);
        if (worker.result)
        {
            msg_info() << "Loaded " << fieldData->GetClassName() << " '" << arrayName
                       << "' with " << array->GetNumberOfTuples() << " entries";
            storage[arrayName] = std::move(worker.result);
        }
    };

    if (numComponents == 1)
    {
        ScalarDataWorker worker{*this, arrayName};
        dispatch(worker);
        return;
    }

    if (numComponents > 9)
    {
        msg_warning() << fieldData->GetClassName() << " array '" << arrayName
            << "' has " << numComponents << " components (max supported: 9)";
        return;
    }

    MultiComponentDataWorker worker{*this, arrayName, numComponents};
    dispatch(worker);
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

        auto* cellData  = dataSet->GetCellData();
        auto* pointData = dataSet->GetPointData();

        for (const auto& arrayName : d_cellDataNames.getValue())
            loadDataArrayByName(cellData, arrayName, m_cellData);

        for (const auto& arrayName : d_pointDataNames.getValue())
            loadDataArrayByName(pointData, arrayName, m_pointData);

        return true;
    }

    return false;
}

void BaseVTKLoader::doClearBuffers()
{
    auto clearMap = [&](auto& map) {
        for (const auto& [name, data] : map)
            this->removeData(data.get());
        map.clear();
    };

    clearMap(m_cellData);
    clearMap(m_pointData);
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
