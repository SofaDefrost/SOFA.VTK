#pragma once
#include <SOFAVTK/config.h>
#include <sofa/core/loader/MeshLoader.h>

namespace sofavtk
{

struct SOFAVTK_API UnstructuredGridVTKLoader : sofa::core::loader::MeshLoader
{
    SOFA_CLASS(UnstructuredGridVTKLoader, sofa::core::loader::MeshLoader);

private:

    bool doLoad() override;
    void doClearBuffers() override;
};

}
