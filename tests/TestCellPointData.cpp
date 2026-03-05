#include <gtest/gtest.h>
#include <sofa/testing/BaseSimulationTest.h>
#include <sofa/vtk/UnstructuredGridVTKLoader.h>
#include <sofa/core/objectmodel/Data.h>
#include <sofa/type/Vec.h>
#include <sofa/type/vector.h>
#include <filesystem>

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
    std::string m_fixturePath;
    std::unique_ptr<SceneInstance> m_scene;

    void doSetUp() override
    {
        m_fixturePath =
            (std::filesystem::temp_directory_path() / "sofa_vtk_test_fixture.vtu")
                .string();
        writeTestFixture(m_fixturePath);
    }

    sofavtk::UnstructuredGridVTKLoader* makeLoader(
        const std::vector<std::string>& cellNames = {},
        const std::vector<std::string>& pointNames = {})
    {
        m_scene = std::make_unique<SceneInstance>();
        auto loader =
            sofa::core::objectmodel::New<sofavtk::UnstructuredGridVTKLoader>();
        m_scene->root->addObject(loader);

        loader->d_filename.setValue(m_fixturePath);
        loader->d_cellDataNames.setValue(
            sofa::type::vector<std::string>(cellNames.begin(), cellNames.end()));
        loader->d_pointDataNames.setValue(
            sofa::type::vector<std::string>(pointNames.begin(), pointNames.end()));

        m_scene->initScene();
        return loader.get();
    }
};

TEST_F(CellPointDataTest, CellData_ScalarFloat)
{
    auto* loader = makeLoader({"pressure"});
    const auto* v = getVec<float>(loader, "pressure");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 4u);
    EXPECT_FLOAT_EQ((*v)[0], 1.0f);
    EXPECT_FLOAT_EQ((*v)[1], 2.0f);
    EXPECT_FLOAT_EQ((*v)[2], 3.0f);
    EXPECT_FLOAT_EQ((*v)[3], 4.0f);
}

TEST_F(CellPointDataTest, CellData_ScalarInt)
{
    auto* loader = makeLoader({"material_id"});
    const auto* v = getVec<int>(loader, "material_id");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 4u);
    EXPECT_EQ((*v)[0], 10);
    EXPECT_EQ((*v)[1], 20);
    EXPECT_EQ((*v)[2], 30);
    EXPECT_EQ((*v)[3], 40);
}

TEST_F(CellPointDataTest, CellData_Vec3Double)
{
    using Vec3d = sofa::type::Vec<3, double>;
    auto* loader = makeLoader({"fiber_direction"});
    const auto* v = getVec<Vec3d>(loader, "fiber_direction");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 4u);

    EXPECT_DOUBLE_EQ((*v)[0][0], 1.0);
    EXPECT_DOUBLE_EQ((*v)[0][1], 0.0);
    EXPECT_DOUBLE_EQ((*v)[0][2], 0.0);

    EXPECT_DOUBLE_EQ((*v)[1][0], 0.0);
    EXPECT_DOUBLE_EQ((*v)[1][1], 1.0);
    EXPECT_DOUBLE_EQ((*v)[1][2], 0.0);

    EXPECT_DOUBLE_EQ((*v)[2][0], 0.0);
    EXPECT_DOUBLE_EQ((*v)[2][1], 0.0);
    EXPECT_DOUBLE_EQ((*v)[2][2], 1.0);

    EXPECT_NEAR((*v)[3][0], 0.577, 1e-6);
    EXPECT_NEAR((*v)[3][1], 0.577, 1e-6);
    EXPECT_NEAR((*v)[3][2], 0.577, 1e-6);
}

TEST_F(CellPointDataTest, CellData_Int8)
{
    auto* loader = makeLoader({"int8_data"});
    const auto* v = getVec<signed char>(loader, "int8_data");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 4u);
    EXPECT_EQ(static_cast<int>((*v)[0]), 1);
    EXPECT_EQ(static_cast<int>((*v)[1]), 2);
    EXPECT_EQ(static_cast<int>((*v)[2]), 3);
    EXPECT_EQ(static_cast<int>((*v)[3]), 4);
}

TEST_F(CellPointDataTest, CellData_Int32)
{
    auto* loader = makeLoader({"int32_data"});
    const auto* v = getVec<int>(loader, "int32_data");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 4u);
    EXPECT_EQ((*v)[0], 100);
    EXPECT_EQ((*v)[1], 200);
    EXPECT_EQ((*v)[2], 300);
    EXPECT_EQ((*v)[3], 400);
}

TEST_F(CellPointDataTest, CellData_Int64)
{
    auto* loader = makeLoader({"int64_data"});
    const auto* v = getVec<long long>(loader, "int64_data");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 4u);
    EXPECT_EQ((*v)[0], 10000LL);
    EXPECT_EQ((*v)[1], 20000LL);
    EXPECT_EQ((*v)[2], 30000LL);
    EXPECT_EQ((*v)[3], 40000LL);
}

TEST_F(CellPointDataTest, PointData_ScalarDouble)
{
    auto* loader = makeLoader({}, {"temperature"});
    const auto* v = getVec<double>(loader, "temperature");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 5u);
    EXPECT_DOUBLE_EQ((*v)[0], 0.1);
    EXPECT_DOUBLE_EQ((*v)[1], 0.2);
    EXPECT_DOUBLE_EQ((*v)[2], 0.3);
    EXPECT_DOUBLE_EQ((*v)[3], 0.4);
    EXPECT_DOUBLE_EQ((*v)[4], 0.5);
}

TEST_F(CellPointDataTest, PointData_Vec3Double)
{
    using Vec3d = sofa::type::Vec<3, double>;
    auto* loader = makeLoader({}, {"velocity"});
    const auto* v = getVec<Vec3d>(loader, "velocity");
    ASSERT_NE(v, nullptr);
    ASSERT_EQ(v->size(), 5u);

    EXPECT_DOUBLE_EQ((*v)[0][0], 1.0);
    EXPECT_DOUBLE_EQ((*v)[0][1], 0.0);
    EXPECT_DOUBLE_EQ((*v)[0][2], 0.0);

    EXPECT_DOUBLE_EQ((*v)[3][0], 2.0);
    EXPECT_DOUBLE_EQ((*v)[3][1], 0.0);
    EXPECT_DOUBLE_EQ((*v)[3][2], 0.0);

    EXPECT_DOUBLE_EQ((*v)[4][0], 0.0);
    EXPECT_DOUBLE_EQ((*v)[4][1], 2.0);
    EXPECT_DOUBLE_EQ((*v)[4][2], 0.0);
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
    EXPECT_EQ(pressure->size(), 4u);

    const auto* matId = getVec<int>(loader, "material_id");
    ASSERT_NE(matId, nullptr);
    EXPECT_EQ(matId->size(), 4u);

    const auto* temp = getVec<double>(loader, "temperature");
    ASSERT_NE(temp, nullptr);
    EXPECT_EQ(temp->size(), 5u);
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
    EXPECT_EQ(matId->size(), 4u);
}

// temperature is point data; requesting it as cell data must not load it
TEST_F(CellPointDataTest, CellVsPoint_Separation)
{
    auto* loader = makeLoader({"temperature"}, {});
    EXPECT_EQ(loader->findData("temperature"), nullptr);
}
