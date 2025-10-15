#pragma once
#include <SOFAVTK/BaseVTKLoader.h>

namespace sofavtk
{

struct SOFAVTK_API PolyDataVTKLoader : BaseVTKLoader
{
    SOFA_CLASS(PolyDataVTKLoader, BaseVTKLoader);

protected:

    vtkSmartPointer<vtkDataSet> getDataSet(
        const sofa::core::objectmodel::DataFileName& fileName) override;
};

}
