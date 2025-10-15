#pragma once
#include <sofa/vtk/BaseVTKLoader.h>

namespace sofavtk
{

struct SOFA_VTK_API PolyDataVTKLoader : BaseVTKLoader
{
    SOFA_CLASS(PolyDataVTKLoader, BaseVTKLoader);

protected:

    vtkSmartPointer<vtkDataSet> getDataSet(
        const sofa::core::objectmodel::DataFileName& fileName) override;
};

}
