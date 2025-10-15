#include <SOFAVTK/PolyDataVTKLoader.h>
#include <sofa/core/ObjectFactory.h>

namespace sofavtk
{

void registerPolyDataVTKLoader(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Mesh loader")
        .add< PolyDataVTKLoader >());
}

bool PolyDataVTKLoader::doLoad()
{
    return false;
}

void PolyDataVTKLoader::doClearBuffers() {}

}  // namespace sofavtk
