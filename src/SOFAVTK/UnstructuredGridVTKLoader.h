#pragma once
#include <SOFAVTK/BaseVTKLoader.h>

namespace sofavtk
{

struct SOFAVTK_API UnstructuredGridVTKLoader : BaseVTKLoader
{
    SOFA_CLASS(UnstructuredGridVTKLoader, BaseVTKLoader);

protected:

    vtkSmartPointer<vtkDataSet> getDataSet(
        const sofa::core::objectmodel::DataFileName& fileName) override;

};

}
