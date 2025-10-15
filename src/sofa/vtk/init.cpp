#include <sofa/vtk/init.h>
#include <sofa/core/ObjectFactory.h>

namespace sofavtk
{

extern void registerUnstructuredGridVTKLoader(sofa::core::ObjectFactory* factory);
extern void registerPolyDataVTKLoader(sofa::core::ObjectFactory* factory);

void initializePlugin() 
{
    static bool first = true;
    if (first) {
        first = false;
        // Register components here
    }
}

}

extern "C" 
{
    SOFA_VTK_API void initExternalModule()
    {
        sofavtk::initializePlugin();
    }

    SOFA_VTK_API const char* getModuleName()
    {
        return sofavtk::MODULE_NAME;
    }

    SOFA_VTK_API const char* getModuleVersion()
    {
        return sofavtk::MODULE_VERSION;
    }

    SOFA_VTK_API const char* getModuleLicense()
    {
        return "LGPL";
    }

    SOFA_VTK_API const char* getModuleDescription()
    {
        return "SOFA plugin using VTK to read meshes";
    }

    SOFA_VTK_API void registerObjects(sofa::core::ObjectFactory* factory)
    {
        sofavtk::registerUnstructuredGridVTKLoader(factory);
        sofavtk::registerPolyDataVTKLoader(factory);
    }
}
