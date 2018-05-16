#ifndef CGAL_LEVEL_OF_DETAIL_BUILDING_PARTITION_CREATOR_H
#define CGAL_LEVEL_OF_DETAIL_BUILDING_PARTITION_CREATOR_H

#if defined(WIN32) || defined(_WIN32) 
#define PSR "\\"
#else 
#define PSR "/" 
#endif

// STL includes.
#include <map>
#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>

// CGAL includes.
#include <CGAL/utils.h>

// New CGAL includes.
#include <CGAL/Mylog/Mylog.h>
#include <CGAL/Level_of_detail_enum.h>
#include <CGAL/Container/Level_of_detail_container.h>
#include <CGAL/Regularizer/Level_of_detail_polygonizer_jean_philippe.h>
#include <CGAL/Buildings/Level_of_detail_building_roof_face_validator.h>
#include <CGAL/Buildings/Associaters/Level_of_detail_building_partition_vote_based_plane_associater.h>

namespace CGAL {

	namespace LOD {

		template<class KernelTraits, class InputContainer, class InputBuildings, class InputBuilding>
		class Level_of_detail_building_partition_creator {
            
        public:
            typedef KernelTraits   Kernel;
            typedef InputContainer Input;
            typedef InputBuilding  Building;
            typedef InputBuildings Buildings;

            using FT         = typename Kernel::FT;
            using Point_2    = typename Kernel::Point_2;
            using Point_3    = typename Kernel::Point_3;
            using Segment_2  = typename Kernel::Segment_2;
            using Segment_3  = typename Kernel::Segment_3;

            using Building_iterator = typename Buildings::iterator;

            using Roof              = typename Building::Roof;
            using Data              = typename Building::Data;
            using Envelope_input    = typename Building::Data_triangles;
            using Envelope_element  = typename Building::Data_triangle;
            using Partition_input   = typename Building::Partition_input;
            using Partition_element = typename Building::Partition_element;
            using Associated_planes = typename Roof::Associated_planes;
            
            using Boundary = std::vector<Point_3>;
            using Log      = CGAL::LOD::Mylog;
            using Segments = std::vector<Segment_2>;

            using Data_structure = CGAL::LOD::Level_of_detail_container<Kernel>;
			using Polygonizer    = CGAL::LOD::Level_of_detail_polygonizer_jean_philippe<Kernel, Data_structure>;

            using Container  = typename Data_structure::Container;
            using Containers = typename Data_structure::Containers;
            
            using Polygon                   = typename Container::Polygon;
            using Polygon_vertices_iterator = typename Polygon::Vertex_const_iterator;

            using Plane_associater    = CGAL::LOD::Level_of_detail_building_partition_vote_based_plane_associater<Kernel, Input, Building>;
            using Roof_face_validator = CGAL::LOD::Level_of_detail_building_roof_face_validator<Kernel, Building>;

            Level_of_detail_building_partition_creator(const Input &input, const FT ground_height) :
            m_input(input),
            m_ground_height(ground_height), 
            m_debug(false),
            m_num_intersections(0),
            m_min_face_width(-FT(1))
            { }

            void create(Buildings &buildings) const {
                
                if (buildings.size() == 0) return;
				for (Building_iterator bit = buildings.begin(); bit != buildings.end(); ++bit) {

                    Building &building = bit->second;
					if (building.is_valid) process_building(building);
                }
            }

            void set_number_of_intersections(const size_t new_value) {

                assert(new_value > 0);
                m_num_intersections = new_value;
            }

            void set_min_face_width(const FT new_value) {
                
                assert(new_value > FT(0));
                m_min_face_width = new_value;
            }

        private:
            const Input &m_input;

            const FT   m_ground_height;
            const bool m_debug;
            
            size_t m_num_intersections;
            FT     m_min_face_width;

            Roof_face_validator m_roof_face_validator;
            
            void process_building(Building &building) const {
                
                const Partition_input &partition_input = building.partition_input;
                if (partition_input.size() < 3) {
                    
                    building.is_valid = false;
                    return;
                }

                Data_structure data_structure;
                Segments &segments = building.partition_segments;

                apply_polygonizer(segments, data_structure);
                update_roofs(data_structure, building);
            }

            void apply_polygonizer(Segments &segments, Data_structure &data_structure) const {

                assert(m_num_intersections > 0);
                assert(m_min_face_width > FT(0));

				Polygonizer polygonizer;
				polygonizer.make_silent(true);

				polygonizer.set_number_of_intersections(m_num_intersections);
				polygonizer.set_min_face_width(m_min_face_width);

				polygonizer.polygonize(segments, data_structure);
            }

            void update_roofs(const Data_structure &data_structure, Building &building) const {

                Roof roof;
                building.clear_roofs();
                
                Boundary &boundary           = roof.boundary;
                const Containers &containers = data_structure.containers();

                for (size_t i = 0; i < containers.size(); ++i) {
					
                    boundary.clear();
                    const Polygon &polygon = containers[i].polygon;

                    for (Polygon_vertices_iterator vit = polygon.vertices_begin(); vit != polygon.vertices_end(); ++vit) {
						const Point_2 &p = *vit;

                        const FT x = p.x();
                        const FT y = p.y();
                        const FT z = building.height + m_ground_height;

                        boundary.push_back(Point_3(x, y, z));
                    }

                    if (m_roof_face_validator.is_valid_roof_face(building, boundary, true))
                        building.roofs.push_back(roof);
				}

                const FT reference_height = building.roofs_min_height + FT(1) / FT(2);
                Plane_associater plane_associater(m_input, building, reference_height);

                for (size_t i = 0; i < building.roofs.size(); ++i)
                    plane_associater.find_associated_planes(i, building.roofs[i].is_plane_index, building.roofs[i].associated_planes);

                if (m_debug) save_polygons(data_structure);
            }

            void save_polygons(const Data_structure &data_structure) const {

                const Containers &containers = data_structure.containers();
                assert(containers.size() > 0);

                Log exporter;
                exporter.save_polygons<Containers, Polygon, Kernel>(containers, "tmp" + std::string(PSR) + "lod_2" + std::string(PSR) + "polygonizer_debug");
            }
        };
    }
}

#endif // CGAL_LEVEL_OF_DETAIL_BUILDING_PARTITION_CREATOR_H