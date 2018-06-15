#ifndef CGAL_LEVEL_OF_DETAIL_COVERAGE_H
#define CGAL_LEVEL_OF_DETAIL_COVERAGE_H

// STL includes.
#include <map>
#include <string>
#include <vector>

// CGAL includes.
/*
#include <CGAL/number_utils.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_tree.h> */

// LOD includes.
/*
#include <CGAL/Level_of_detail/Enumerations.h> */

namespace CGAL {

	namespace Level_of_detail {

		namespace LOD = CGAL::Level_of_detail;

		template<class InputKernel, class InputContainer, class InputReconstruction>
		class Coverage {

		public:
			using Kernel    	 = InputKernel;
			using Container 	 = InputContainer;
			using Reconstruction = InputReconstruction;

			using FT = typename Kernel::FT;

			/*
			typedef typename Reconstruction::Mesh 			   Mesh;
			typedef typename Reconstruction::Mesh_facet_colors Mesh_facet_colors;

			using Vertex_handle   = typename Mesh::Vertex_handle;
			using Halfedge_handle = typename Mesh::Halfedge_const_handle;
			using Facet_handle    = typename Mesh::Facet_const_handle;
			using Faces 	      = std::vector< std::vector<Facet_handle> >;

			typedef typename Kernel::Point_3    Point_3;
			typedef typename Kernel::Vector_3   Vector_3;
			typedef typename Kernel::Plane_3    Plane_3;
			typedef typename Kernel::Point_2    Point_2;
			typedef typename Kernel::Triangle_2 Triangle_2;

			using Index   = int;
			using Indices = std::vector<Index>;

			typedef LOD::Level_of_detail_utils_simple<Kernel> Simple_utils;
			typedef LOD::Level_of_detail_building_interior<Kernel, Container> Roofs_points_selection_strategy;
			typedef LOD::Level_of_detail_building_boundary<Kernel, Container> Walls_points_selection_strategy;			
			typedef LOD::Level_of_detail_selector<Kernel, Roofs_points_selection_strategy> Roofs_points_selector;
			typedef LOD::Level_of_detail_selector<Kernel, Walls_points_selection_strategy> Walls_points_selector;

            typedef CGAL::AABB_face_graph_triangle_primitive<Mesh> AB_primitive;
			typedef CGAL::AABB_traits<Kernel, AB_primitive> 	   AB_traits;
			typedef CGAL::AABB_tree<AB_traits> 					   AB_tree;
			typedef typename AB_tree::Point_and_primitive_id 	   AB_point_and_primitive_id;

            struct Bounding_box { Point_3 bbmin, bbmax; };
			using Bounding_boxes = std::vector<Bounding_box>; */

			Coverage(const Container &input, const Reconstruction &reconstruction) : 
			m_input(input), 
			m_reconstruction(reconstruction),
			m_roofs_coverage(-FT(1)), 
			m_walls_coverage(-FT(1)), 
			m_coverage(-FT(1)),
            m_num_roofs_inliers(-1), 
			m_num_walls_inliers(-1),
            m_num_roofs_points(-1), 
			m_num_walls_points(-1),
			m_distance_threshold(-FT(1)) 
			{ }

			/*
            void estimate(const FT distance_threshold) {
                m_distance_threshold = distance_threshold;

				const FT roofs_coverage = get_roofs_coverage();
				const FT walls_coverage = get_walls_coverage();
                const FT total_coverage = get_total_coverage();

				m_roofs_coverage = roofs_coverage;
				m_walls_coverage = walls_coverage;
				m_coverage 	     = total_coverage;
            }

			FT get_for_roofs() const {
				
				CGAL_precondition(m_roofs_coverage >= FT(0));
				return m_roofs_coverage;
			}

			FT get_for_walls() const {
				
				CGAL_precondition(m_walls_coverage >= FT(0));
				return m_walls_coverage;
			}

            FT get() {
				
				CGAL_precondition(m_coverage >= FT(0));
				return m_coverage;
			} */

        private:
			const Container 	 &m_input;
			const Reconstruction &m_reconstruction;

			FT m_roofs_coverage;
			FT m_walls_coverage;
			FT m_coverage;
			
            int m_num_roofs_inliers;
			int m_num_walls_inliers;

            int m_num_roofs_points;
			int m_num_walls_points;

            FT m_distance_threshold;

			/*
            FT get_roofs_coverage() {
                
                Mesh roofs_mesh;
				get_roofs(roofs_mesh);

                Indices roofs_point_indices;
				get_roofs_points(roofs_point_indices);

                std::vector<Point_3> roofs_translated_points;
				get_translated_points(roofs_mesh, roofs_point_indices, roofs_translated_points);
				
                const size_t num_roofs_points   = roofs_translated_points.size();
                const size_t num_roofs_outliers = compute_number_of_roofs_outliers(roofs_translated_points);
                const size_t num_roofs_inliers  = num_roofs_points - num_roofs_outliers;

                m_num_roofs_points  = static_cast<int>(num_roofs_points);
                m_num_roofs_inliers = static_cast<int>(num_roofs_inliers);

                const FT roofs_coverage = (static_cast<FT>(num_roofs_inliers) / static_cast<FT>(num_roofs_points)) * FT(100);
                return roofs_coverage;
            }

            FT get_walls_coverage() {
                
                Mesh walls_mesh;
				get_walls(walls_mesh);

                Indices walls_point_indices;
				get_walls_points(walls_point_indices);

                std::vector<Point_3> walls_translated_points;
				get_translated_points(walls_mesh, walls_point_indices, walls_translated_points);
                
                const size_t num_walls_points   = walls_translated_points.size();
                const size_t num_walls_outliers = compute_number_of_walls_outliers(walls_mesh, walls_translated_points);
                const size_t num_walls_inliers  = num_walls_points - num_walls_outliers;

                m_num_walls_points  = static_cast<int>(num_walls_points);
                m_num_walls_inliers = static_cast<int>(num_walls_inliers);

                const FT walls_coverage = (static_cast<FT>(num_walls_inliers) / static_cast<FT>(num_walls_points)) * FT(100);
                return walls_coverage;
            }

            FT get_total_coverage() {

                CGAL_precondition(m_num_roofs_inliers >= 0 && m_num_roofs_points >= 0);
                CGAL_precondition(m_num_walls_inliers >= 0 && m_num_walls_points >= 0);
                
                const int num_total_inliers = m_num_roofs_inliers + m_num_walls_inliers;
                const int num_total_points  = m_num_roofs_points  + m_num_walls_points; 

                const FT total_coverage = (static_cast<FT>(num_total_inliers) / static_cast<FT>(num_total_points)) * FT(100);
                return total_coverage;
            }

            inline void get_roofs(Mesh &roofs) {
				m_lods.get_roofs(roofs);
			}

			inline void get_walls(Mesh &walls) {
				m_lods.get_walls(walls);
			}

            void get_roofs_points(Indices &roofs_point_indices) {
				roofs_point_indices.clear();
				
				Roofs_points_selector selector;
				selector.select_elements(m_input, std::back_inserter(roofs_point_indices));
			}

			void get_walls_points(Indices &walls_point_indices) {
				walls_point_indices.clear();
				
				Walls_points_selector selector;
				selector.select_elements(m_input, std::back_inserter(walls_point_indices));
			}

			void get_translated_points(const Mesh &mesh, const Indices &point_indices, std::vector<Point_3> &translated_points) {

				const Vector_3 translation = get_translation(mesh, point_indices);
				translate_points(translation, point_indices, translated_points);
			}

			Vector_3 get_translation(const Mesh &mesh, const Indices &point_indices) {

				const Point_3 mesh_barycentre   = get_mesh_barycentre(mesh);
				const Point_3 points_barycentre = get_points_barycentre(point_indices);

				return Vector_3(mesh_barycentre, points_barycentre);
			}

            Point_3 get_mesh_barycentre(const Mesh &mesh) {

				FT num_points = FT(0);
				FT x = FT(0), y = FT(0), z = FT(0);

				for (typename Mesh::Vertex_const_iterator vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit, num_points += FT(1)) {
					const Point_3 &p = vit->point();

					x += p.x();
					y += p.y();
					z += p.z();
				}
				x /= num_points;
				y /= num_points;
				z /= num_points;

				return Point_3(x, y, z);
			}

			Point_3 get_points_barycentre(const Indices &point_indices) {

				FT num_points = FT(0);
				FT x = FT(0), y = FT(0), z = FT(0);

				for (size_t i = 0; i < point_indices.size(); ++i, num_points += FT(1)) {
					const Point_3 &p = m_input.point(point_indices[i]);

					x += p.x();
					y += p.y();
					z += p.z();
				}
				x /= num_points;
				y /= num_points;
				z /= num_points;

				return Point_3(x, y, z);
			}

            void translate_points(const Vector_3 &translation, const Indices &point_indices, std::vector<Point_3> &translated_points) {

				translated_points.clear();
				translated_points.resize(point_indices.size());

				for (size_t i = 0; i < point_indices.size(); ++i) {
					const Point_3 &p = m_input.point(point_indices[i]);
					
					const FT z = p.z() - translation.z();
					translated_points[i] = Point_3(p.x(), p.y(), z);
				}
			}

            size_t compute_number_of_roofs_outliers(const std::vector<Point_3> &points) {

				Faces roofs_faces;
				m_lods.get_roofs_faces(roofs_faces);

				Bounding_boxes boxes;
				compute_bounding_boxes(roofs_faces, boxes);

				size_t num_roofs_outliers = 0;
				for (size_t i = 0; i < points.size(); ++i) {
					const Point_3 &query = points[i];
					
					std::vector<int> box_indices;
					find_bounding_boxes(query, boxes, box_indices);
					
					if (box_indices.empty()) {
						++num_roofs_outliers; continue;
					}

					bool found = false;
					for (size_t j = 0; j < box_indices.size(); ++j) {

						const Plane_3 plane = get_plane_from_bounding_box(boxes[box_indices[j]].bbmin, boxes[box_indices[j]].bbmax);
						const Point_3 projected = plane.projection(query);

						const int face_index = find_face(projected, roofs_faces[box_indices[j]]);

						if (face_index >= 0) {
							found = true; break;
						}
					}

					if (!found) {
						++num_roofs_outliers; continue;
					}
				}
                return num_roofs_outliers;
			}

			void compute_bounding_boxes(const Faces &faces, Bounding_boxes &boxes) {

				CGAL_precondition(!faces.empty());
				boxes.resize(faces.size());

				std::vector<Point_3> points;
				for (size_t i = 0; i < faces.size(); ++i) {
					
					get_points(faces[i], points);
					m_simple_utils.compute_bounding_box_in_3d(boxes[i].bbmin, boxes[i].bbmax, points);

					CGAL_precondition(boxes[i].bbmin.z() == boxes[i].bbmax.z());
				}
			}

			void get_points(const std::vector<Facet_handle> &faces, std::vector<Point_3> &points) {
				CGAL_precondition(!faces.empty());

				points.clear();
				for (size_t i = 0; i < faces.size(); ++i) {
					
					Halfedge_handle he = faces[i]->halfedge();
					points.push_back(he->vertex()->point());

					he = he->next();
					points.push_back(he->vertex()->point());

					he = he->next();
					points.push_back(he->vertex()->point());
				}
			}

			void find_bounding_boxes(const Point_3 &query, const Bounding_boxes &boxes, std::vector<int> &box_indices) {
				
				CGAL_precondition(!boxes.empty());
				CGAL_precondition(box_indices.empty());

				for (size_t i = 0; i < boxes.size(); ++i)
					if (is_inside_box(query, boxes[i].bbmin, boxes[i].bbmax)) 
						box_indices.push_back(static_cast<int>(i));
			}

			bool is_inside_box(const Point_3 &query, const Point_3 &minp, const Point_3 &maxp) {

				if (query.x() > minp.x() && query.x() < maxp.x() && 
					query.y() > minp.y() && query.y() < maxp.y()) return true;

				return false;
			}

            Plane_3 get_plane_from_bounding_box(const Point_3 &minp, const Point_3 &maxp) {
				CGAL_precondition(minp.z() == maxp.z());

				const Point_3 a = minp;
				const Point_3 b = Point_3(maxp.x(), minp.y(), minp.z());
				const Point_3 c = maxp;

				return Plane_3(a, b, c);
			}

			int find_face(const Point_3 &query, const std::vector<Facet_handle> &faces) {
				CGAL_precondition(!faces.empty());

				for (size_t i = 0; i < faces.size(); ++i)
					if (belongs_to_face(query, faces[i]))
						return static_cast<int>(i);

				return -1;
			}

			bool belongs_to_face(const Point_3 &query, const Facet_handle &fh) {

				std::vector<Point_3> points;
				get_points_from_face_handle(fh, points);

				CGAL_precondition(points.size() == 3);
				CGAL_precondition(points[0].z() == points[1].z() && points[1].z() == points[2].z());

				const Triangle_2 triangle = Triangle_2(
					Point_2(points[0].x(), points[0].y()),
					Point_2(points[1].x(), points[1].y()),
					Point_2(points[2].x(), points[2].y())
					);

				const Point_2 new_query = Point_2(query.x(), query.y());

				if (triangle.has_on_bounded_side(new_query) || triangle.has_on_boundary(new_query)) return true;
				return false;
			}

			void get_points_from_face_handle(const Facet_handle &fh, std::vector<Point_3> &points) {
				
				points.clear();
				Halfedge_handle he = fh->halfedge();

				const Point_3 p1 = he->vertex()->point();

				he = he->next();
				const Point_3 p2 = he->vertex()->point();

				he = he->next();
				const Point_3 p3 = he->vertex()->point();

				points.push_back(p1);
				points.push_back(p2);
				points.push_back(p3);
			}

            size_t compute_number_of_walls_outliers(const Mesh &mesh, const std::vector<Point_3> &points) {

				Mesh tmp_mesh = mesh;
				for (Vertex_handle vh = tmp_mesh.vertices_begin(); vh != tmp_mesh.vertices_end(); ++vh) {

					Point_3 &p = vh->point();
					p = Point_3(p.x(), p.y(), FT(0));
				}

                CGAL_precondition(m_distance_threshold > FT(0));
				AB_tree aabb_tree(faces(tmp_mesh).first, faces(tmp_mesh).second, tmp_mesh);

				size_t num_walls_outliers = 0;
				Point_3 closest_point;

				for (size_t i = 0; i < points.size(); ++i) {
					
					const Point_3 query = Point_3(points[i].x(), points[i].y(), FT(0));
					closest_point = aabb_tree.closest_point(query);

					const FT squared_dist = squared_distance(query, closest_point);
					const FT distance = static_cast<FT>(CGAL::sqrt(CGAL::to_double(squared_dist)));

					if (distance > m_distance_threshold) {
                        ++num_walls_outliers; continue;
                    }
				}
                return num_walls_outliers;
            }

            Plane_3 get_plane_from_face_handle(const Facet_handle &fh) {
				
				std::vector<Point_3> points;
				get_points_from_face_handle(fh, points);

				CGAL_precondition(points.size() == 3);
				return Plane_3(points[0], points[1], points[2]);
			} */
        };

    } // Level_of_detail

} // CGAL

#endif // CGAL_LEVEL_OF_DETAIL_COVERAGE_H