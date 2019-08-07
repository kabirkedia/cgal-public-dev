// STL includes.
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <list>

// CGAL includes.
#include <CGAL/property_map.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Shape_regularization.h>

#include "saver_segments_2.h"

// Typedefs.
// typedef CGAL::Exact_predicates_inexact_constructions_kernel Traits;
// typedef CGAL::Simple_cartesian<double> Traits;
typedef CGAL::Exact_predicates_exact_constructions_kernel Traits;

using Segment_2 = typename Traits::Segment_2;
using Point_2 = typename Traits::Point_2;
using FT = typename Traits::FT;

using Input_range = std::vector<Segment_2>;
using Segment_map = CGAL::Identity_property_map<Segment_2>;

using Neighbor_query = CGAL::Regularization::Delaunay_neighbor_query_2<Traits, Input_range, Segment_map>;
using Regularization_type_angles = CGAL::Regularization::Angle_regularization_2<Traits, Input_range, Segment_map>;
using Regularization_type_ordinates = CGAL::Regularization::Ordinate_regularization_2<Traits, Input_range, Segment_map>;

using Shape_regularization_angles = CGAL::Regularization::Shape_regularization
  <Traits, Input_range, Neighbor_query, Regularization_type_angles>;
using Shape_regularization_ordinates = CGAL::Regularization::Shape_regularization
  <Traits, Input_range, Neighbor_query, Regularization_type_ordinates>;
using Parallel_groups = CGAL::Regularization::Parallel_groups_2<Traits, Input_range, Segment_map>;

using Saver = CGAL::Regularization::Saver_segments_2<Traits>;


int main() {

  const Point_2 a = Point_2(0.2, 1.0);
  const Point_2 b = Point_2(0.2, 0.2);

  const Point_2 c = Point_2(0.2364941756489, 1.0451701975863);
  const Point_2 d = Point_2(0.2410972444872, 1.5975384581787);

  const Point_2 e = Point_2(0.6,1.6);
  const Point_2 f = Point_2(0.6392626989976, 1.4134157046479);

  const Point_2 g = Point_2(0.2, 2.0);
  const Point_2 h = Point_2(0.595533545034, 2.0463376699101);

  Input_range input_range;
  input_range.push_back(Segment_2(a, b));
  input_range.push_back(Segment_2(c, d));
  input_range.push_back(Segment_2(e, f));
  input_range.push_back(Segment_2(g, h));

  std::cout.precision(15);

  std::cout << std::endl;
  std::cout << "BEFORE:" << std::endl;
  for (const auto& segment : input_range)
    std::cout << segment << std::endl;
  std::cout << std::endl;

  Saver saver;
  saver.save_segments(input_range, "test_4_segments_before");

  Neighbor_query neighbor_query(input_range);
  std::vector<std::size_t> vec;
  vec.resize(input_range.size());
  std::iota(vec.begin(), vec.end(), 0);
  neighbor_query.add_group(vec);
  
  const FT bound_angles = FT(5);
  Regularization_type_angles regularization_type_angles(input_range, bound_angles);
  regularization_type_angles.add_group(vec);

  Shape_regularization_angles shape_regularization_angles(
    input_range, neighbor_query, regularization_type_angles);

  shape_regularization_angles.regularize();

  std::size_t counter_angl = 0;
  std::cout << "After angles: " << std::endl;
  for (const auto& segment : input_range) {
    const FT point1 = segment.source().x() + segment.source().y();
    const FT point2 = segment.target().x() + segment.target().y();
    const FT both = point1 + point2;
    std::cout << ++counter_angl << "). " << both << std::endl;
  }
  std::cout << "Number of modified segments angles: " << regularization_type_angles.number_of_modified_segments() << std::endl;

  // Regularization for ordinates:
  std::vector <std::vector <std::size_t>> parallel_groups;
  regularization_type_angles.parallel_groups(std::back_inserter(parallel_groups));

  std::cout << "parallel_groups.size() = " << parallel_groups.size() << std::endl;

  const FT bound_ordinates = FT(0.1);
  Regularization_type_ordinates regularization_type_ordinates(input_range, bound_ordinates);

  neighbor_query.clear();
  for(const auto & group : parallel_groups) {
    neighbor_query.add_group(group);
    regularization_type_ordinates.add_group(group);
  }

  Shape_regularization_ordinates Shape_regularization_ordinates(
    input_range, neighbor_query, regularization_type_ordinates);
  Shape_regularization_ordinates.regularize();
  
  std::cout << "AFTER:" << std::endl;
  for (const auto& segment : input_range)
    std::cout << segment << std::endl;
  std::cout << std::endl;
  saver.save_segments(input_range, "test_4_segments_after"); 

  std::size_t counter_ord = 0;
  std::cout << "After ordinates: " << std::endl;
  for (const auto& segment : input_range) {
    const FT point1 = segment.source().x() + segment.source().y();
    const FT point2 = segment.target().x() + segment.target().y();
    const FT both = point1 + point2;
    std::cout << ++counter_ord << "). " << both << std::endl;
  }
  std::cout << "Number of modified segments ordinates: " << regularization_type_ordinates.number_of_modified_segments() << std::endl;


  return EXIT_SUCCESS;
}
