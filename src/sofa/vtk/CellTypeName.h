#pragma once

#include <sofa/vtk/config.h>

#include <string>
#include <vtkCellType.h>

namespace sofavtk
{

std::string SOFA_VTK_API getCellTypeName(VTKCellType cellType);

}
