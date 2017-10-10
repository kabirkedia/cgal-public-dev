#ifndef CGAL_LEVEL_OF_DETAIL_PROJECTOR_H
#define CGAL_LEVEL_OF_DETAIL_PROJECTOR_H

// STL includes.
#include <map>
#include <cassert>

namespace CGAL {

	namespace LOD {

		template<class KernelTraits, class InputContainer, class PlanesMapping, class OutputContainer>
		class Level_of_detail_projector {		

		public:
			virtual int project(const InputContainer &, const PlanesMapping &, const typename KernelTraits::Plane_3 &, OutputContainer &) = 0;
			virtual ~Level_of_detail_projector() { }
		};

		template<class KernelTraits, class InputContainer, class PlanesMapping, class OutputContainer>
		class Level_of_detail_simple_projector : public Level_of_detail_projector<KernelTraits, InputContainer, PlanesMapping, OutputContainer> {

		public:
			typedef KernelTraits Traits;

			typedef typename Traits::Plane_3 Plane;
			typedef typename Traits::Point_2 Point_2;
			typedef typename Traits::Point_3 Point_3;

			typedef InputContainer  Container;
			typedef PlanesMapping   Planes;
			typedef OutputContainer Output;

			using Const_iterator = typename Planes::const_iterator;

			int project(const Container &input, const Planes &planes, const typename Traits::Plane_3 &ground, Output &projected) override { 

				projected.clear();

				auto number_of_projected_points = 0;
				for (Const_iterator it = planes.begin(); it != planes.end(); ++it) {

					for (size_t i = 0; i < (*it).second.size(); ++i) {

						const auto index = (*it).second[i];
						const auto point = input.point(index);	

						// Maybe better to remove Z coordinate completely? But it will mean that I only project to XY plane
						// instead of an arbitrary ground plane.
						// Be careful with projection. May bug!
						const auto projected_point = ground.projection(point);
						projected[index] = Point_2(projected_point.x(), projected_point.y()); // here I assume that Z coordinate = 0!

						++number_of_projected_points;
					}
				}
				assert(number_of_projected_points == static_cast<int>(projected.size()));
				return number_of_projected_points;
			}
		};
	}
}

#endif // CGAL_LEVEL_OF_DETAIL_PROJECTOR_H