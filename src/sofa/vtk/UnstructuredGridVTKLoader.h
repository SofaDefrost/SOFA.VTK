#pragma once
#include <sofa/vtk/BaseVTKLoader.h>

namespace sofavtk
{

struct SOFA_VTK_API UnstructuredGridVTKLoader : BaseVTKLoader
{
    SOFA_CLASS(UnstructuredGridVTKLoader, BaseVTKLoader);

protected:

    vtkSmartPointer<vtkDataSet> getDataSet(
        const sofa::core::objectmodel::DataFileName& fileName) override;
};

}
