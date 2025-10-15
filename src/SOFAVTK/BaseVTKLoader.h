#pragma once
#include <SOFAVTK/config.h>
#include <sofa/core/loader/MeshLoader.h>
#include <vtkDataSet.h>
#include <vtkSmartPointer.h>

namespace sofavtk
{

struct SOFAVTK_API BaseVTKLoader : sofa::core::loader::MeshLoader
{
    SOFA_ABSTRACT_CLASS(BaseVTKLoader, sofa::core::loader::MeshLoader);

private:

    bool doLoad() final;
    void doClearBuffers() final;

protected:

    virtual vtkSmartPointer<vtkDataSet> getDataSet(const sofa::core::objectmodel::DataFileName& fileName) = 0;
};

}
