#ifndef CGAL_LEVEL_OF_DETAIL_BUILDING_ROOF_ESTIMATOR_HULL_STRATEGY_H
#define CGAL_LEVEL_OF_DETAIL_BUILDING_ROOF_ESTIMATOR_HULL_STRATEGY_H

// STL includes.
#include <map>
#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>

// CGAL includes.
#include <CGAL/utils.h>
#include <CGAL/number_utils.h>
#include <CGAL/convex_hull_2.h>

// New CGAL includes.
#include <CGAL/Level_of_detail_enum.h>

namespace CGAL {

	namespace LOD {

		// Main class.
		template<class KernelTraits, class ContainerInput, class BuildingInput>
		class Level_of_detail_building_roof_estimator_hull_strategy {
            
        public:
            typedef KernelTraits   Kernel;
            typedef ContainerInput Input;
            typedef BuildingInput  Building;

            using FT       = typename Kernel::FT;
            using Point_2  = typename Kernel::Point_2;
			using Point_3  = typename Kernel::Point_3;
            using Plane_3  = typename Kernel::Plane_3;
            using Points_2 = std::vector<Point_2>;
			using Points_3 = std::vector<Point_3>;

            using Roof = typename Building::Roof;

            Level_of_detail_building_roof_estimator_hull_strategy(const Input &input) : m_input(input) { }

            void estimate_roof(const Points_3 &roof_points, const Plane_3 &, Building &building) const {
                
				// Set convex hull input and find convex hull.
				if (roof_points.size() < 2) return;
				Points_2 tmp_roof_points(roof_points.size()), result;

				for (size_t i = 0; i < roof_points.size(); ++i) tmp_roof_points[i] = Point_2(roof_points[i].x(), roof_points[i].y());
				CGAL::convex_hull_2(tmp_roof_points.begin(), tmp_roof_points.end(), std::back_inserter(result));

				// Match convex hull points to roof points.
				Points_3 boundary;
				set_roof_boundary(result, roof_points, boundary);

				// Set estimated roof.
				Roof roof;
                roof.boundary = boundary;
                building.roofs.push_back(roof);
            }

			void set_alpha(const FT) { }

			bool is_face_based() const {
				return false;
			}

			std::string name() const {
				return "hull";
			}

        private:
            const Input &m_input;

			void set_roof_boundary(const Points_2 &result, const Points_3 &roof_points, Points_3 &boundary) const {
				
				boundary.clear();
				boundary.resize(result.size());

				for (size_t i = 0; i < result.size(); ++i) {
				
					FT z;
					for (size_t j = 0; j < roof_points.size(); ++j) {
						if (roof_points[j].x() == result[i].x() && roof_points[j].y() == result[i].y()) {
							
							z = roof_points[j].z();
							break;
						}
					}
					boundary[i] = Point_3(result[i].x(), result[i].y(), z);
				}
			}
        };
    }
}

#endif // CGAL_LEVEL_OF_DETAIL_BUILDING_ROOF_ESTIMATOR_HULL_STRATEGY_H