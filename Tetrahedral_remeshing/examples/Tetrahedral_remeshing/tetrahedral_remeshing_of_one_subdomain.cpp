#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <iostream>
#include <fstream>

#include <CGAL/Triangulation_3.h>

#include <CGAL/Tetrahedral_remeshing/Remeshing_triangulation_3.h>
#include <CGAL/tetrahedral_remeshing.h>

#include <CGAL/Random.h>

#include "tetrahedral_remeshing_io.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

typedef CGAL::Tetrahedral_remeshing::Remeshing_triangulation_3<K> Remeshing_triangulation;

template<typename T3>
bool generate_input_two_subdomains(const std::size_t nbv, T3& tr)
{
  CGAL::Random rng;
  std::cout << "CGAL Random seed = " << CGAL::get_default_random().get_seed() << std::endl;

  typedef typename T3::Point Point;
  while (tr.number_of_vertices() < nbv)
    tr.insert(Point(rng.get_double(-1., 1.), rng.get_double(-1., 1.), rng.get_double(-1., 1.)));

  const typename T3::Geom_traits::Plane_3
    plane(Point(0, 0, 0), Point(0, 1, 0), Point(0, 0, 1));

  for (typename T3::Cell_handle c : tr.finite_cell_handles())
  {
    if (plane.has_on_positive_side(
      CGAL::centroid(c->vertex(0)->point(), c->vertex(1)->point(),
        c->vertex(2)->point(), c->vertex(3)->point())))
      c->set_subdomain_index(1);
    else
      c->set_subdomain_index(2);
  }
  CGAL_assertion(tr.is_valid(true));

  std::string filename("data/triangulation_two_subdomains.binary.cgal");
  std::ofstream out(filename, std::ios_base::out | std::ios_base::binary);
  save_binary_triangulation(out, tr);

  return (!out.bad());
}

struct Cells_of_subdomain
{
private:
  const int m_subdomain;

public:
  Cells_of_subdomain(const int& subdomain)
    : m_subdomain(subdomain)
  {}

  bool operator()(Remeshing_triangulation::Cell_handle c) const
  {
    return m_subdomain == c->subdomain_index();
  }
};

int main(int argc, char* argv[])
{
  CGAL::get_default_random() = CGAL::Random(1586522498);

  const float target_edge_length = (argc > 1) ? atof(argv[1]) : 0.1f;

  Remeshing_triangulation tr;
  generate_input_two_subdomains(1000, tr);

  CGAL::tetrahedral_adaptive_remeshing(tr, target_edge_length,
      CGAL::parameters::cell_selector(Cells_of_subdomain(2)));

  std::ofstream ofile("output.binary.cgal", std::ios::out);
  save_binary_triangulation(ofile, tr);

  return EXIT_SUCCESS;
}

