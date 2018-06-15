#ifndef CGAL_LEVEL_OF_DETAIL_DELAUNAY_NEIGHBOURS_GRAPH_BUILDER_H
#define CGAL_LEVEL_OF_DETAIL_DELAUNAY_NEIGHBOURS_GRAPH_BUILDER_H

// STL includes.
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <vector>
#include <utility>

// CGAL includes.
#include <CGAL/number_utils.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

// LOD includes.
#include <CGAL/Level_of_detail/Regularization/Segment_regularizer_2/Segment_regularizer_parameters.h>

#include <CGAL/Level_of_detail/Regularization/Segment_regularizer_2/Regular_segment.h>
#include <CGAL/Level_of_detail/Regularization/Segment_regularizer_2/Regular_segment_sampler.h>

namespace CGAL {

    namespace Level_of_detail {

        namespace LOD = CGAL::Level_of_detail;

        template<class InputKernel, class NeighboursGraphData>
        class Delaunay_neighbours_graph_builder {

        public:
            using Kernel                = InputKernel;
            using Neighbours_graph_data = NeighboursGraphData;

            using FT      = typename Kernel::FT;
            using Point   = typename Kernel::Point_2;
            using Segment = typename Kernel::Segment_2;

            using Regular_segment  = LOD::Regular_segment<Kernel>;
            using Regular_segments = std::vector<Regular_segment *>;
            using Orientations     = std::vector<FT>;

            using Considered_potential  = std::pair<size_t, size_t>;
            using Considered_potentials = std::set<Considered_potential>;

            using Parameters      = LOD::Segment_regularizer_parameters<FT>;
            using Segment_sampler = LOD::Regular_segment_sampler<Kernel>;

            using Point_with_index  = std::pair<Point, size_t>;
            using Points            = std::vector<Point_with_index>;
            using Point_segment_map = std::map<size_t, size_t>;

            using VB = CGAL::Triangulation_vertex_base_with_info_2<size_t, Kernel>;
            using DS = CGAL::Triangulation_data_structure_2<VB>;
            using DT = CGAL::Delaunay_triangulation_2<Kernel, DS>;

            using Edge_iterator = typename DT::Finite_edges_iterator;
            using Edge          = typename DT::Edge;

            using FT_triplet  = typename Neighbours_graph_data::FT_triplet;
            using Int_triplet = typename Neighbours_graph_data::Int_triplet;

            Delaunay_neighbours_graph_builder(
                const Regular_segments &segments,
                const Orientations &max_orientations,
                const Parameters &parameters) : 
                m_segments(segments), 
                m_max_orientations(max_orientations), 
                m_parameters(parameters) 
                { }

            void build_graph_data(Neighbours_graph_data &graph_data) {

                // Sample regularly all segments.
                sample_segments();

                // Build Delaunay triangulation from sample points.
                build_delaunay_triangulation();

                // Compute spatial proximity.
                estimate_proximity(graph_data);
            }

        private:
            const Regular_segments &m_segments;
            const Orientations     &m_max_orientations;
            const Parameters       &m_parameters;

            Points            m_points;
            Point_segment_map m_points_to_segments;
            DT                m_dt;

            void sample_segments() {
                CGAL_precondition(m_segments.size() > 0);

                const Segment_sampler segment_sampler(m_segments);
                segment_sampler.sample(m_points, m_points_to_segments);
            }

            void build_delaunay_triangulation() {
                
                m_dt.clear();
                m_dt.insert(m_points.begin(), m_points.end());
            }

            void estimate_proximity(Neighbours_graph_data &graph_data) {
                
                CGAL_precondition(m_dt.number_of_vertices() > 0 && m_dt.number_of_faces() > 0);
                graph_data.clear();

                Considered_potentials considered_potentials;
                for (Edge_iterator eit = m_dt.finite_edges_begin(); eit != m_dt.finite_edges_end(); ++eit) {
                    const Edge &edge = *eit;

                    const size_t e_i = edge.first->vertex((edge.second + 1) % 3)->info();
                    const size_t e_j = edge.first->vertex((edge.second + 2) % 3)->info();

                    CGAL_precondition(e_i < m_points_to_segments.size());
                    CGAL_precondition(e_j < m_points_to_segments.size());

                    const size_t i = m_points_to_segments[e_i];
                    const size_t j = m_points_to_segments[e_j];

                    if (i == j) continue;

                    const Considered_potential p_ij = (i < j ? std::make_pair(i, j) : std::make_pair(j, i));
		            if (considered_potentials.find(p_ij) != considered_potentials.end()) continue;
		            considered_potentials.insert(p_ij);

                    CGAL_precondition(i < m_segments.size());
                    CGAL_precondition(j < m_segments.size());

                    const Regular_segment *s_i = m_segments[i];
                    const Regular_segment *s_j = m_segments[j];

                    const FT mes_ij    = s_i->get_orientation() - s_j->get_orientation();
                    const double mes90 = std::floor(CGAL::to_double(mes_ij / FT(90)));

                    const FT to_lower = FT(90) *  static_cast<FT>(mes90)          - mes_ij;
                    const FT to_upper = FT(90) * (static_cast<FT>(mes90) + FT(1)) - mes_ij;

                    const FT mu_ij = m_parameters.lambda();
                    CGAL_precondition(mu_ij >= FT(0) && mu_ij <= FT(1));

                    const FT  t_ij = CGAL::abs(to_lower) < CGAL::abs(to_upper) ? to_lower : to_upper;
                    int      r_ij;

                    if (CGAL::abs(to_lower) < CGAL::abs(to_upper))
                        r_ij = ((90 * static_cast<int>(mes90)) % 180 == 0 ? 0 : 1);
                    else
                        r_ij = ((90 * static_cast<int>(mes90 + 1.0)) % 180 == 0 ? 0 : 1);

                    if (
                        (r_ij == 0 && m_parameters.optimize_parallelizm())  || 
                        (r_ij == 1 && m_parameters.optimize_orthogonality()) ) {

                        CGAL_precondition(i < m_max_orientations.size());
                        CGAL_precondition(j < m_max_orientations.size());

                        if (CGAL::abs(t_ij) < m_max_orientations[i] + m_max_orientations[j]) {

                            graph_data.get_mus().push_back(       FT_triplet(i, j, mu_ij));
                            graph_data.get_targets().push_back(   FT_triplet(i, j,  t_ij));
                            graph_data.get_relations().push_back(Int_triplet(i, j,  r_ij));
                        }
                    }
                }
            }
        };

    } // Level_of_detail
    
} // CGAL

#endif // CGAL_LEVEL_OF_DETAIL_DELAUNAY_NEIGHBOURS_GRAPH_BUILDER_H