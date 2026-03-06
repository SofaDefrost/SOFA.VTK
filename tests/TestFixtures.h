#pragma once

#include <string>
#include <filesystem>

// Returns the absolute path to the pre-generated test fixture.
//
// Geometry: 22 points, 10 cells — three spatial zones:
//   Zone A  z=0       2 quads (cells 0-1) + 2 triangles (cells 2-3)
//   Zone B  z=1→2     2 hexahedra         (cells 4-5)
//   Zone C  z=2→3     4 tetrahedra        (cells 6-9)
//
// All data values are functions of coordinates, verifiable by selecting
// an element in ParaView and computing the expected value directly:
//
//   Cell data (10 entries):
//     pressure        float32   centroid z    [0.0, 0.0, 0.0, 0.0, 1.5, 1.5, 2.25, 2.25, 2.25, 2.25]
//     material_id     int32     zone id       [1, 1, 1, 1, 2, 2, 3, 3, 3, 3]
//     fiber_direction float64   normalised centroid vector
//     int8_data       int8      cell index    [1 .. 10]
//     int32_data      int32     index * 100   [100 .. 1000]
//     int64_data      int64     index * 10000 [10000 .. 100000]
//     too_many        float64   10 components, all 0  (>9-component guard)
//
//   Point data (22 entries):
//     temperature     float64   x + y + z     [0.0 .. 5.0]
//     velocity        float64   (x, y, z)     radial from origin

inline std::string fixtureVtuPath()
{
    // __FILE__ is this header; fixtures/ sits alongside tests/
    std::filesystem::path here = std::filesystem::path(__FILE__).parent_path();
    return (here / "fixtures" / "test_mesh.vtu").string();
}
