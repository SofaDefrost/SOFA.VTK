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

    sofa::core::objectmodel::Data<sofa::type::vector<std::string>> d_cellDataNames{
        initData(&d_cellDataNames, "cellDataNames",
                 "Names of cell data arrays to load from the VTK file")};

    sofa::core::objectmodel::Data<sofa::type::vector<std::string>> d_pointDataNames{
        initData(&d_pointDataNames, "pointDataNames",
                 "Names of point data arrays to load from the VTK file")};

private:

    bool doLoad() final;
    void doClearBuffers() final;

    void loadCellDataArrayByName(vtkSmartPointer<vtkDataSet> dataset, const std::string& arrayName);
    void loadPointDataArrayByName(vtkSmartPointer<vtkDataSet> dataset, const std::string& arrayName);

    std::map<std::string, std::unique_ptr<sofa::core::objectmodel::BaseData>> m_cellData;
    std::map<std::string, std::unique_ptr<sofa::core::objectmodel::BaseData>> m_pointData;

protected:

    virtual vtkSmartPointer<vtkDataSet> getDataSet(const sofa::core::objectmodel::DataFileName& fileName) = 0;
};

}
