#include <SOFAVTK/init.h>
#include <sofa/core/ObjectFactory.h>

namespace sofavtk
{

extern void registerUnstructuredGridVTKLoader(sofa::core::ObjectFactory* factory);

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
    SOFAVTK_API void initExternalModule() 
    {
        sofavtk::initializePlugin();
    }

    SOFAVTK_API const char* getModuleName() 
    {
        return sofavtk::MODULE_NAME;
    }

    SOFAVTK_API const char* getModuleVersion() 
    {
        return sofavtk::MODULE_VERSION;
    }

    SOFAVTK_API const char* getModuleLicense() 
    {
        return "LGPL";
    }

    SOFAVTK_API const char* getModuleDescription() 
    {
        return "SOFA plugin using VTK to read meshes";
    }

    SOFAVTK_API void registerObjects(sofa::core::ObjectFactory* factory)
    {
        sofavtk::registerUnstructuredGridVTKLoader(factory);
    }
}
