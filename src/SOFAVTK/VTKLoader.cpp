#include <SOFAVTK/VTKLoader.h>
#include <sofa/core/ObjectFactory.h>

namespace sofavtk
{

void registerVTKLoader(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Mesh loader for the VTK/VTU file format.")
        .add< VTKLoader >());
}

bool VTKLoader::doLoad()
{
    return false;
}

void VTKLoader::doClearBuffers()
{

}

} // namespace sofavtk
