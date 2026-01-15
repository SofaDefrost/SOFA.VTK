#pragma once
#include <sofa/vtk/config.h>
#include <sofa/core/loader/MeshLoader.h>
#include <vtkDataSet.h>
#include <vtkSmartPointer.h>
#include <cstdint>
#include <memory>
#include <map>

namespace sofavtk
{

struct SOFA_VTK_API BaseVTKLoader : sofa::core::loader::MeshLoader
{
    SOFA_ABSTRACT_CLASS(BaseVTKLoader, sofa::core::loader::MeshLoader);

    /// Names of VTK cell data arrays to load
    sofa::core::objectmodel::Data<sofa::type::vector<std::string>> d_cellDataNames{
        initData(&d_cellDataNames, "cellDataNames",
                 "Names of cell data arrays to load from the VTK file")};

private:

    bool doLoad() final;
    void doClearBuffers() final;

    /// Load a cell data array from VTK dataset
    template<typename DataType, int NumComponents>
    void loadCellDataArray(vtkSmartPointer<vtkDataSet> dataset,
                          const std::string& arrayName);

    /// Dispatch cell data loading based on VTK type and number of components
    void loadCellDataArrayByName(vtkSmartPointer<vtkDataSet> dataset, const std::string& arrayName);

    /// Unified storage for all dynamically created Data objects
    std::map<std::string, std::unique_ptr<sofa::core::objectmodel::BaseData>> m_cellData;

protected:

    virtual vtkSmartPointer<vtkDataSet> getDataSet(const sofa::core::objectmodel::DataFileName& fileName) = 0;
};

}
