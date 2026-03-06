#include <gtest/gtest.h>
#include <sofa/testing/BaseSimulationTest.h>
#include <sofa/vtk/UnstructuredGridVTKLoader.h>
#include <sofa/core/objectmodel/Data.h>
#include <sofa/type/Vec.h>
#include <sofa/type/vector.h>

#include "TestFixtures.h"

namespace
{

// Returns a pointer to the inner vector of a typed Data field on obj, or
// nullptr if the field is missing or has a different type.
template <typename T>
const sofa::type::vector<T>* getVec(sofa::core::objectmodel::Base* obj,
                                    const std::string& name)
{
    auto* base = obj->findData(name);
    if (!base)
        return nullptr;
    auto* typed =
        dynamic_cast<sofa::core::objectmodel::Data<sofa::type::vector<T>>*>(base);
    return typed ? &typed->getValue() : nullptr;
}

}  // namespace

class CellPointDataTest : public sofa::testing::BaseSimulationTest
{
protected:
    std::unique_ptr<SceneInstance> m_scene;

    sofavtk::UnstructuredGridVTKLoader* makeLoader(
        const std::vector<std::string>& cellNames = {},
        const std::vector<std::string>& pointNames = {})
    {
        m_scene = std::make_unique<SceneInstance>();
        auto loader =
            sofa::core::objectmodel::New<sofavtk::UnstructuredGridVTKLoader>();
        m_scene->root->addObject(loader);

        loader->d_filename.setValue(fixtureVtuPath());
        loader->d_cellDataNames.setValue(
            sofa::type::vector<std::string>(cellNames.begin(), cellNames.end()));
        loader->d_pointDataNames.setValue(
            sofa::type::vector<std::string>(pointNames.begin(), pointNames.end()));

        m_scene->initScene();
        return loader.get();
    }
};

// Fixture has 10 cells (2 quads + 2 tris + 2 hexas + 4 tets)
// and 22 points.

// pressure = centroid z: 0.0 for surface cells, 1.5 for hexas, 2.25 for tets
TEST_F(CellPointDataTest, CellData_ScalarFloat)
{
    auto* loader = makeLoader({"pressure"});
    const auto* v = getVec<float>(loader, "pressure");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 10u);
    // Zone A: centroid z = 0.0
    EXPECT_FLOAT_EQ((*v)[0], 0.0f);
    EXPECT_FLOAT_EQ((*v)[3], 0.0f);
    // Zone B: centroid z = 1.5
    EXPECT_FLOAT_EQ((*v)[4], 1.5f);
    EXPECT_FLOAT_EQ((*v)[5], 1.5f);
    // Zone C: centroid z = 2.25
    EXPECT_FLOAT_EQ((*v)[6], 2.25f);
    EXPECT_FLOAT_EQ((*v)[9], 2.25f);
}

// material_id = zone id: 1=surface, 2=hexa, 3=tet
TEST_F(CellPointDataTest, CellData_ScalarInt)
{
    auto* loader = makeLoader({"material_id"});
    const auto* v = getVec<int>(loader, "material_id");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 10u);
    EXPECT_EQ((*v)[0], 1);   // quad
    EXPECT_EQ((*v)[2], 1);   // triangle
    EXPECT_EQ((*v)[4], 2);   // hexa
    EXPECT_EQ((*v)[6], 3);   // tet
}

// fiber_direction = normalised centroid vector
TEST_F(CellPointDataTest, CellData_Vec3Double)
{
    using Vec3d = sofa::type::Vec<3, double>;
    auto* loader = makeLoader({"fiber_direction"});
    const auto* v = getVec<Vec3d>(loader, "fiber_direction");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 10u);

    // Every entry must be a unit vector
    for (std::size_t i = 0; i < v->size(); ++i)
    {
        const double mag = (*v)[i].norm();
        EXPECT_NEAR(mag, 1.0, 1e-9) << "cell " << i << " fiber_direction is not unit";
    }

    // Cell 0 centroid = (0.5, 0.5, 0) → normalised (1/√2, 1/√2, 0)
    const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    EXPECT_NEAR((*v)[0][0], inv_sqrt2, 1e-9);
    EXPECT_NEAR((*v)[0][1], inv_sqrt2, 1e-9);
    EXPECT_NEAR((*v)[0][2], 0.0,       1e-9);

    // Zone C tets: centroid z >> x,y so z-component dominates
    EXPECT_GT((*v)[6][2], 0.9);
}

TEST_F(CellPointDataTest, CellData_Int8)
{
    auto* loader = makeLoader({"int8_data"});
    const auto* v = getVec<signed char>(loader, "int8_data");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 10u);
    // int8_data = cell index + 1
    for (std::size_t i = 0; i < v->size(); ++i)
        EXPECT_EQ(static_cast<int>((*v)[i]), static_cast<int>(i + 1));
}

TEST_F(CellPointDataTest, CellData_Int32)
{
    auto* loader = makeLoader({"int32_data"});
    const auto* v = getVec<int>(loader, "int32_data");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 10u);
    // int32_data = (cell index + 1) * 100
    for (std::size_t i = 0; i < v->size(); ++i)
        EXPECT_EQ((*v)[i], static_cast<int>((i + 1) * 100));
}

TEST_F(CellPointDataTest, CellData_Int64)
{
    auto* loader = makeLoader({"int64_data"});
    const auto* v = getVec<long long>(loader, "int64_data");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 10u);
    // int64_data = (cell index + 1) * 10000
    for (std::size_t i = 0; i < v->size(); ++i)
        EXPECT_EQ((*v)[i], static_cast<long long>((i + 1) * 10000));
}

// temperature = x + y + z per point, range 0.0 (P0 at origin) to 5.0 (P19)
TEST_F(CellPointDataTest, PointData_ScalarDouble)
{
    auto* loader = makeLoader({}, {"temperature"});
    const auto* v = getVec<double>(loader, "temperature");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 22u);
    EXPECT_DOUBLE_EQ((*v)[0],  0.0);   // P0  (0,0,0)
    EXPECT_DOUBLE_EQ((*v)[1],  1.0);   // P1  (1,0,0)
    EXPECT_DOUBLE_EQ((*v)[5],  3.0);   // P5  (2,1,0)
    EXPECT_DOUBLE_EQ((*v)[19], 5.0);   // P19 (2,1,2)
    EXPECT_DOUBLE_EQ((*v)[21], 4.0);   // P21 (0.5,0.5,3)
}

// velocity = (x, y, z) per point — direction and magnitude encode position
TEST_F(CellPointDataTest, PointData_Vec3Double)
{
    using Vec3d = sofa::type::Vec<3, double>;
    auto* loader = makeLoader({}, {"velocity"});
    const auto* v = getVec<Vec3d>(loader, "velocity");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 22u);

    // P0 (0,0,0) → zero velocity
    EXPECT_DOUBLE_EQ((*v)[0][0], 0.0);
    EXPECT_DOUBLE_EQ((*v)[0][1], 0.0);
    EXPECT_DOUBLE_EQ((*v)[0][2], 0.0);

    // P5 (2,1,0) → velocity = coordinates
    EXPECT_DOUBLE_EQ((*v)[5][0], 2.0);
    EXPECT_DOUBLE_EQ((*v)[5][1], 1.0);
    EXPECT_DOUBLE_EQ((*v)[5][2], 0.0);

    // P21 (0.5,0.5,3) → velocity = coordinates
    EXPECT_DOUBLE_EQ((*v)[21][0], 0.5);
    EXPECT_DOUBLE_EQ((*v)[21][1], 0.5);
    EXPECT_DOUBLE_EQ((*v)[21][2], 3.0);
}

TEST_F(CellPointDataTest, MissingArrayName)
{
    auto* loader = makeLoader({"nonexistent"});
    EXPECT_EQ(loader->findData("nonexistent"), nullptr);
}

TEST_F(CellPointDataTest, TooManyComponents)
{
    auto* loader = makeLoader({"too_many"});
    EXPECT_EQ(loader->findData("too_many"), nullptr);
}

TEST_F(CellPointDataTest, MultipleArraysAtOnce)
{
    auto* loader = makeLoader({"pressure", "material_id"}, {"temperature"});

    const auto* pressure = getVec<float>(loader, "pressure");
    ASSERT_NE(pressure, nullptr);
    EXPECT_EQ(pressure->size(), 10u);

    const auto* matId = getVec<int>(loader, "material_id");
    ASSERT_NE(matId, nullptr);
    EXPECT_EQ(matId->size(), 10u);

    const auto* temp = getVec<double>(loader, "temperature");
    ASSERT_NE(temp, nullptr);
    EXPECT_EQ(temp->size(), 22u);
}

TEST_F(CellPointDataTest, Reload)
{
    auto* loader = makeLoader({"pressure"});
    ASSERT_NE(getVec<float>(loader, "pressure"), nullptr);
    EXPECT_EQ(loader->findData("material_id"), nullptr);

    loader->d_cellDataNames.setValue({"material_id"});
    loader->load();

    EXPECT_EQ(loader->findData("pressure"), nullptr);
    const auto* matId = getVec<int>(loader, "material_id");
    ASSERT_NE(matId, nullptr);
    EXPECT_EQ(matId->size(), 10u);
}

// temperature is point data; requesting it as cell data must not load it
TEST_F(CellPointDataTest, CellVsPoint_Separation)
{
    auto* loader = makeLoader({"temperature"}, {});
    EXPECT_EQ(loader->findData("temperature"), nullptr);
}
