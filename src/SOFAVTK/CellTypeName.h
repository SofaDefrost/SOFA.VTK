#pragma once

#include <SOFAVTK/config.h>

#include <string>
#include <vtkCellType.h>

namespace sofavtk
{

std::string SOFAVTK_API getCellTypeName(VTKCellType cellType);

}
