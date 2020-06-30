// Works with an exact kernel, too.

#include <cmath>
#include <vector>
#include <cassert>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Barycentric_coordinates_2/analytic_coordinates_2.h>

using Kernel  = CGAL::Exact_predicates_inexact_constructions_kernel;
using FT      = typename Kernel::FT;
using Point_2 = typename Kernel::Point_2;

int main() {

  const std::vector<Point_2> vertices = {
    Point_2(0, 0),
    Point_2(1, 0),
    Point_2(0, 1)
  };

  std::vector<FT> tri_coordinates;
  std::vector<FT>  mv_coordinates;

  const FT step  = FT(1) / FT(100);
  const FT scale = FT(50);

  std::size_t count = 0;
  const FT limit = scale * step;
  const FT epsilon = FT(1) / FT(std::pow(10.0, 14.0));

  for (FT x = step; x < limit; x += step) {
    for (FT y = step; y < limit; y += step) {
      const Point_2 query(x, y);

      CGAL::Barycentric_coordinates::triangle_coordinates_2(
        vertices[0], vertices[1], vertices[2], query, std::back_inserter(tri_coordinates));
      CGAL::Barycentric_coordinates::mean_value_coordinates_2(
        vertices, query, std::back_inserter(mv_coordinates));
      assert(
        (tri_coordinates[count + 0] - mv_coordinates[count + 0]) < epsilon &&
        (tri_coordinates[count + 1] - mv_coordinates[count + 1]) < epsilon &&
        (tri_coordinates[count + 2] - mv_coordinates[count + 2]) < epsilon );
      count += 3;
    }
  }

  std::cout << "test_mv_triangle: PASSED" << std::endl;
  return EXIT_SUCCESS;
}
