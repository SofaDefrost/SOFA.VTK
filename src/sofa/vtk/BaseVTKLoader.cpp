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

// long and unsigned long have platform-dependent sizes (32-bit on Windows, 64-bit on Linux/Mac).
// Remap them to the fixed-size type of the same width so the SOFA Data type is consistent
// and predictable regardless of platform.
template<typename T> struct CanonicalLong                { using type = T;                                                              };
template<> struct CanonicalLong<long>                    { using type = std::conditional_t<sizeof(long) == 4, int, long long>;          };
template<> struct CanonicalLong<unsigned long>           { using type = std::conditional_t<sizeof(unsigned long) == 4, unsigned int, unsigned long long>; };

template<typename T>
using CanonicalLong_t = typename CanonicalLong<T>::type;

struct ScalarCellDataWorker
{
    sofavtk::BaseVTKLoader& loader;
    const std::string& arrayName;
    std::unique_ptr<sofa::core::objectmodel::BaseData> result;
    vtkIdType numLoaded = 0;

    template<typename ArrayT>
    void operator()(ArrayT* array)
    {
        using T = CanonicalLong_t<vtk::GetAPIType<ArrayT>>;

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

struct MultiComponentCellDataWorker
{
    sofavtk::BaseVTKLoader& loader;
    const std::string& arrayName;
    int numComponents;
    std::unique_ptr<sofa::core::objectmodel::BaseData> result;
    vtkIdType numLoaded = 0;

    template<typename ArrayT>
    void operator()(ArrayT* array)
    {
        using T = CanonicalLong_t<vtk::GetAPIType<ArrayT>>;
        dispatchN<T, 2>(array);
    }

private:
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
        dataPtr->setHelp("Cell data loaded from VTK file");

        {
            auto accessor = sofa::helper::getWriteOnlyAccessor(*dataPtr);
            auto& vec = accessor.wref();
            vec.resize(array->GetNumberOfTuples());
            vtkIdType i = 0;
            for (const auto tuple : vtk::DataArrayTupleRange<N>(array))
            {
                for (int c = 0; c < N; ++c)
                    vec[i][c] = tuple[c];
                ++i;
            }
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

    // Explicitly supported value types. Each maps to a fixed-size C++ type that SOFA's
    // type system reliably handles. long and unsigned long are included and remapped via
    // CanonicalLong_t to int/long long based on their size on the current platform.
    using SupportedTypes = vtkTypeList::Create<
        float, double,
        int, unsigned int,
        long, unsigned long,
        long long, unsigned long long>;
    using Dispatcher = vtkArrayDispatch::DispatchByValueType<SupportedTypes>;

    if (numComponents == 1)
    {
        ScalarCellDataWorker worker{*this, arrayName};
        if (!Dispatcher::Execute(array, worker))
            worker(array);
        if (worker.result)
        {
            msg_info() << "Loaded cell data '" << arrayName << "' with " << worker.numLoaded << " entries";
            m_cellData[arrayName] = std::move(worker.result);
        }
        return;
    }

    if (numComponents > 9)
    {
        msg_warning() << "Cell data array '" << arrayName
            << "' has " << numComponents << " components (max supported: 9)";
        return;
    }

    MultiComponentCellDataWorker worker{*this, arrayName, numComponents};
    if (!Dispatcher::Execute(array, worker))
        worker(array);
    if (worker.result)
    {
        msg_info() << "Loaded cell data '" << arrayName << "' with " << worker.numLoaded << " entries";
        m_cellData[arrayName] = std::move(worker.result);
    }
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
