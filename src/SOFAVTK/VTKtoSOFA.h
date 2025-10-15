#pragma once

#include <SOFAVTK/config.h>
#include <sofa/type/Vec.h>
#include <sofa/type/vector.h>

#include <vtkDataSet.h>
#include <vtkSmartPointer.h>

namespace sofavtk
{

void SOFAVTK_API extractPoints(
    sofa::type::vector<sofa::type::Vec3>& positions,
    vtkSmartPointer<vtkDataSet> dataSet);

}
