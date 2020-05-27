#include "include/utils.h"
#include "include/Saver.h"
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Shape_regularization/regularize_segments.h>

// Typedefs.
using Kernel    = CGAL::Simple_cartesian<double>;
using FT        = typename Kernel::FT;
using Segment_2 = typename Kernel::Segment_2;
using Segments  = std::vector<Segment_2>;

using NQ    = CGAL::Shape_regularization::Segments::Delaunay_neighbor_query_2<Kernel, Segments>;
using AR    = CGAL::Shape_regularization::Segments::Angle_regularization_2<Kernel, Segments>;
using Saver = CGAL::Shape_regularization::Examples::Saver<Kernel>;

int main(int argc, char *argv[]) {

  // If we want to save the result in a file, we save it in a path.
  std::string path = "";
  if (argc > 1) path = argv[1];
  Saver saver;

  // Initialize 100 near-orthogonal segments.
  Segments segments;
  CGAL::Shape_regularization::Examples::
  create_example_angles(segments);

  // Save input segments.
  if (path != "") {
    const std::string full_path = path + "regularize_100_segments_angles_before";
    saver.export_eps_segments(segments, full_path, FT(1));
  }

  // Angle regularization.
  const FT max_angle_2 = FT(40);

  NQ neighbor_query(segments);
  AR angle_regularization(
    segments, CGAL::parameters::max_angle(max_angle_2));
  CGAL::Shape_regularization::Segments::regularize_segments(
    segments, neighbor_query, angle_regularization);

  std::cout << "* number of modified segments = " <<
    angle_regularization.number_of_modified_segments() << std::endl;

  // Save regularized segments.
  if (path != "") {
    const std::string full_path = path + "regularize_100_segments_angles_after";
    saver.export_eps_segments(segments, full_path, FT(1));
  }
}
