#ifndef CGAL_LEVEL_OF_DETAIL_SEGMENT_REGULARIZER_TREE_H
#define CGAL_LEVEL_OF_DETAIL_SEGMENT_REGULARIZER_TREE_H

// STL includes.
#include <map>
#include <list>
#include <vector>

// CGAL includes.
#include <CGAL/number_utils.h>

// LOD includes.
#include <CGAL/Level_of_detail/Regularization/Segment_regularizer_2/Regular_segment.h>
#include <CGAL/Level_of_detail/Regularization/Segment_regularizer_2/Segment_regularizer_parameters.h>
#include <CGAL/Level_of_detail/Regularization/Segment_regularizer_2/Segment_regularizer_tree_parallel_segments_node.h>

namespace CGAL {

	namespace Level_of_detail {

        namespace LOD = CGAL::Level_of_detail;

        template<class InputKernel, class QPProblemData>
		class Segment_regularizer_tree {

        public:
            using Kernel          = InputKernel;
            using QP_problem_data = QPProblemData;

            using FT     = typename Kernel::FT;
            using Point  = typename Kernel::Point_2;
            using Vector = typename Kernel::Vector_2;

            using Regular_segment             = LOD::Regular_segment<Kernel>;
            using Regular_segments            = std::vector<Regular_segment *>;
            using Parallel_segments_tree_node = LOD::Segment_regularizer_tree_parallel_segments_node<Kernel>;

            using Parallel_segments          = std::map<FT, Parallel_segments_tree_node>;
            using Parallel_segments_iterator = typename Parallel_segments::const_iterator;

            using Orientations = std::vector<FT>;
            using Parameters   = LOD::Segment_regularizer_parameters<FT>;

            using Mus_matrix       = typename QP_problem_data::Mus_matrix;
            using Targets_matrix   = typename QP_problem_data::Targets_matrix;
            using Relations_matrix = typename QP_problem_data::Relations_matrix;

            using Mus_iterator       = typename Mus_matrix::InnerIterator;
            using Targets_iterator   = typename Targets_matrix::InnerIterator;
            using Relations_iterator = typename Relations_matrix::InnerIterator;

            using List_element  = std::list<int>;
            using List_iterator = typename List_element::const_iterator;

            using Segments_to_groups = std::vector<int>;
            using Groups_to_segments = std::map<int, List_element>;

            using Angles          = std::map<int, FT>;
            using Angles_iterator = typename Angles::const_iterator;

            using Subtree_segments_iterator = typename Parallel_segments_tree_node::Parallel_segments_const_iterator;

            Segment_regularizer_tree(Regular_segments &segments, const Orientations &orientations, const QP_problem_data &qp_data, const Parameters &parameters) : 
            m_segments(segments), 
            m_orientations(orientations), 
            m_qp_data(qp_data), 
            m_parameters(parameters) { 

                clear();
                build_tree();
            }

            void apply_new_orientations() {
                for (Parallel_segments_iterator it_ps = m_parallel_segments.begin(); it_ps != m_parallel_segments.end(); ++it_ps) {
                    
                    const FT theta                             = it_ps->first;
                    const Parallel_segments_tree_node &subtree = it_ps->second;

                    // Each group of parallel segments has a normal vector that we compute with alpha.
                    const FT x = static_cast<FT>(cos(CGAL::to_double(theta * static_cast<FT>(CGAL_PI) / FT(180))));
                    const FT y = static_cast<FT>(sin(CGAL::to_double(theta * static_cast<FT>(CGAL_PI) / FT(180))));

                    const Vector v_dir = Vector(x, y);
                    const Vector v_ort = Vector(-v_dir.y(), v_dir.x());
                    
                    const FT a = v_ort.x();
                    const FT b = v_ort.y();

                    // Rotate segments with precision.
                    for (Subtree_segments_iterator it_st = subtree.get_parallel_segments().begin(); it_st != subtree.get_parallel_segments().end(); ++it_st) {
                        Regular_segment *segment_pointer = *it_st;

                        // Compute equation of the supporting line of the rotated segment.
                        const Point &barycentre = segment_pointer->get_barycentre();
                        const FT c = -a * barycentre.x() - b * barycentre.y();

                        segment_pointer->set_orientation(theta - segment_pointer->get_orientation(), a, b, c, v_dir);
                    }
                }
            }

            void clear() {
                m_parallel_segments.clear();
            }

            Parallel_segments &get_parallel_segments() {
                return m_parallel_segments;
            }

        private:
            Parallel_segments  m_parallel_segments;
            Regular_segments  &m_segments;

            const Orientations     &m_orientations;
            const QP_problem_data  &m_qp_data;
            const Parameters       &m_parameters;

            void build_tree() {
                
                // Prepare some data.
                CGAL_precondition(m_segments.size() > 0);
                const int n = static_cast<int>(m_segments.size());

                Segments_to_groups segments_to_groups(n, -1);
                Groups_to_segments groups_to_segments;

                const Targets_matrix   &targets_matrix   = m_qp_data.get_targets_matrix();
                const Relations_matrix &relations_matrix = m_qp_data.get_relations_matrix();

                const FT theta_eps = m_parameters.epsilon();
                CGAL_precondition(theta_eps >= FT(0));


                // Categorize segments.
                int g = 0, p = 0;
                for (int k = 0; k < targets_matrix.outerSize(); ++k) {

                    Targets_iterator     it_targets(  targets_matrix, k);
                    Relations_iterator it_relations(relations_matrix, k);

                    while (it_targets && it_relations) {

                        const int i = it_targets.row();
                        const int j = it_targets.col();
                        const int r = it_relations.value();

                        if (CGAL::abs(m_orientations[n + p]) < m_parameters.tolerance()) {

                            // case-->
                            if (segments_to_groups[i] == -1 && segments_to_groups[j] == -1) {
                                if (r == 0) {
                                    
                                    // Then segments i and j belong to the same group of parallel segments.
                                    // We should create a group of segments, that is initialized with these two individuals.
                                    segments_to_groups[i] = segments_to_groups[j] = g;
                                    groups_to_segments[g].push_back(i);
                                    groups_to_segments[g].push_back(j);
                                    ++g;

                                } else if (r == 1) {
                                    
                                    // The segments i and j are orthogonal.
                                    // We create two different groups of parallel segments.
                                    segments_to_groups[i] = g;
                                    groups_to_segments[g].push_back(i);
                                    segments_to_groups[j] = ++g;
                                    groups_to_segments[g].push_back(j);
                                    ++g;
                                }

                            } 
                            // case--> 
                            else if (segments_to_groups[i] == -1 && segments_to_groups[j] != -1) {
                                if (r == 0) {

                                    // Then segment i is parallel to j, and can be assigned to the same group.
                                    const int g_j = segments_to_groups[j];
                                    segments_to_groups[i] = g_j;
                                    groups_to_segments[g_j].push_back(i);

                                } else if (r == 1) {
                                    
                                    // Then segment i is orthogonal to j, and we should initialize a new group with this segment.
                                    segments_to_groups[i] = g;
                                    groups_to_segments[g].push_back(i);
                                    ++g;
                                }
                            } 
                            // case-->
                            else if (segments_to_groups[i] != -1 && segments_to_groups[j] == -1) {
                                
                                // Symmetrical situation to before.
                                if (r == 0) {

                                    const int g_i = segments_to_groups[i];
                                    segments_to_groups[j] = g_i;
                                    groups_to_segments[g_i].push_back(j);

                                } else if (r == 1) {

                                    segments_to_groups[j] = g;
                                    groups_to_segments[g].push_back(j);
                                    ++g;
                                }
                            } 
                            // case-->
                            else {
                                const int g_i = segments_to_groups[i];
                                const int g_j = segments_to_groups[j];

                                if (g_i != g_j) {
                                    if (r == 0) {
                                        
                                        // Segments i and j have been assigned to different groups, but in fact
                                        // they are parallel and belong to the same group. That's why we merge them.
                                        for (List_iterator it_list = groups_to_segments[g_j].begin(); it_list != groups_to_segments[g_j].end(); ++it_list) {

                                            segments_to_groups[*it_list] = g_i;
                                            groups_to_segments[g_i].push_back(*it_list);
                                        }
                                        groups_to_segments[g_j].clear();

                                    } else if (r == 1) {
                                        // We do nothing here.
                                    }
                                }
                            }
                        }

                        ++p;
			            ++it_targets;
			            ++it_relations;
                    }
                }


                // Prepare for the construction of the regularization tree.
                Angles angles;

                for (size_t i = 0; i < segments_to_groups.size(); ++i) {
		            const int g_i = segments_to_groups[i];

                    if (g_i != -1) {
			            if (angles.find(g_i) == angles.end()) {
                            FT theta = m_segments[i]->get_orientation() + m_orientations[i];

                            if (theta < FT(0)) theta += FT(180);
                            else if (theta > FT(180)) theta -= FT(180);

                            // Check if the angle that seems to be associated to this group of segments is not too close to another value.
                            int g_j = -1;
                            for (Angles_iterator it_angle = angles.begin(); it_angle != angles.end(); ++it_angle)
                                if (CGAL::abs(it_angle->second - theta) < theta_eps) g_j = it_angle->first;

                            if (g_j == -1) angles[g_i] = theta;
                            else {
                                
                                // Merge groups.
                                for (List_iterator it_list = groups_to_segments[g_i].begin(); it_list != groups_to_segments[g_i].end(); ++it_list) {
                                    
                                    segments_to_groups[*it_list] = g_j;
                                    groups_to_segments[g_j].push_back(*it_list);
                                }
                                groups_to_segments[g_i].clear();
                            }
                        }
                    }
                }


                // Try to assign segments whose orientation has not been optimized thanks to the regularization process, to an existing group.
	            for (size_t i = 0; i < segments_to_groups.size(); ++i) {
		            int g_i = segments_to_groups[i];

                    if (g_i == -1) {

			            const FT alpha = m_segments[i]->get_orientation();
                        int        g_j = -1;

                        for (Angles_iterator it_angle = angles.begin(); it_angle != angles.end(); ++it_angle) {
                            const FT alpha_j = it_angle->second;

                            for (int k = -1; k <= 1; ++k) {
                                if (CGAL::abs(alpha_j - alpha + static_cast<FT>(k) * FT(180)) < theta_eps) {

                                    g_j = it_angle->first;
                                    break;
                                }
                            }
                            if (g_j != -1) break;
                        }

                        if (g_j == -1) {
                            
                            g_i = angles.rbegin()->first + 1;
                            angles[g_i] = alpha;

                        } else g_i = g_j;

                        segments_to_groups[i] = g_i;
			            groups_to_segments[g_i].push_back(i);
                    }
                }


                // Build regularization tree.
                for (Angles_iterator it_angle = angles.begin(); it_angle != angles.end(); ++it_angle) 
                    create_parallel_node(angles[it_angle->first]);

                for (size_t i = 0; i < segments_to_groups.size(); ++i) {
                    
                    // If segment s_i is included in a group of parallel segments,
                    // then it should be assigned to a leaf of the regularization tree.
                    assign_to_parallel_node(angles[segments_to_groups[i]], m_segments[i]);
                }
            }

            void create_parallel_node(const FT angle) {

                if (m_parallel_segments.find(angle) == m_parallel_segments.end()) 
                    m_parallel_segments[angle] = Parallel_segments_tree_node();
            }

            void assign_to_parallel_node(const FT angle, Regular_segment *segment_pointer) {

                if (m_parallel_segments.find(angle) != m_parallel_segments.end())
                    m_parallel_segments[angle].add(segment_pointer);
            }
		};

	} // Level_of_detail

} // CGAL

#endif // CGAL_LEVEL_OF_DETAIL_SEGMENT_REGULARIZER_TREE_H