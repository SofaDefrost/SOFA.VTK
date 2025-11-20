#pragma once
#include <sofa/vtk/config.h>
#include <sofa/core/loader/MeshLoader.h>
#include <vtkDataSet.h>
#include <vtkSmartPointer.h>
#include <memory>
#include <map>

namespace sofavtk
{

struct SOFA_VTK_API BaseVTKLoader : sofa::core::loader::MeshLoader
{
    SOFA_ABSTRACT_CLASS(BaseVTKLoader, sofa::core::loader::MeshLoader);

    using Vec3Vector = sofa::type::vector<sofa::type::Vec3>;

    /// Names of VTK cell data arrays (3-component vectors) to load
    sofa::core::objectmodel::Data<sofa::type::vector<std::string>> d_cellVectorDataNames{
        initData(&d_cellVectorDataNames, "cellVectorDataNames",
                 "Names of cell data arrays (Vec3) to load from the VTK file")};

    /// Get a loaded cell vector data by its VTK array name. Returns nullptr if not found.
    sofa::core::objectmodel::Data<Vec3Vector>* getCellVectorData(const std::string& name) const;

private:

    bool doLoad() final;
    void doClearBuffers() final;

    void loadCellVectorData(vtkSmartPointer<vtkDataSet> dataset);

    /// Storage for dynamically created Data objects
    std::map<std::string, std::unique_ptr<sofa::core::objectmodel::Data<Vec3Vector>>> m_cellVectorData;

protected:

    virtual vtkSmartPointer<vtkDataSet> getDataSet(const sofa::core::objectmodel::DataFileName& fileName) = 0;

    virtual void loadVTKData(vtkSmartPointer<vtkDataSet> dataset) {}
};

}
