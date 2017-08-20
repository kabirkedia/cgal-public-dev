#ifndef CURVATURE_FLOW_IMPL_H
#define CURVATURE_FLOW_IMPL_H

#include <utility>
#include <math.h>

#include <CGAL/Polygon_mesh_processing/Weights.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/repair.h>
#include <CGAL/Polygon_mesh_processing/measure.h>

#include <CGAL/property_map.h>
#include <CGAL/iterator.h>
#include <CGAL/boost/graph/Euler_operations.h>
#include <boost/graph/graph_traits.hpp>
#include <boost/foreach.hpp>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>


namespace CGAL {

namespace Polygon_mesh_processing {

namespace internal {



template<typename PolygonMesh, typename VertexPointMap,
         typename CotangentValue = CGAL::internal::Cotangent_value_Meyer_secure<PolygonMesh, VertexPointMap>>
class Cotangent_weight : CotangentValue
{
public:
    Cotangent_weight(PolygonMesh& pmesh_, VertexPointMap vpmap_)
      : CotangentValue(pmesh_, vpmap_)
    {}

    PolygonMesh& pmesh()
    {
      return CotangentValue::pmesh();
    }

    typedef typename boost::graph_traits<PolygonMesh>::halfedge_descriptor   halfedge_descriptor;
    typedef typename boost::graph_traits<PolygonMesh>::vertex_descriptor     vertex_descriptor;
    typedef std::pair<halfedge_descriptor, halfedge_descriptor>              he_pair;

    double operator()(halfedge_descriptor he, he_pair incd_edges)
    {
      vertex_descriptor vs = source(he, pmesh());
      vertex_descriptor vt = target(he, pmesh());
      vertex_descriptor v1 = target(incd_edges.first, pmesh());
      vertex_descriptor v2 = source(incd_edges.second, pmesh());

      return ( CotangentValue::operator()(vs, v1, vt) + CotangentValue::operator()(vs, v2, vt));
    }

};



template<typename PolygonMesh, typename VertexPointMap, typename VertexConstraintMap, typename EdgeConstraintMap, typename GeomTraits>
class Curvature_flow
{

    typedef typename boost::graph_traits<PolygonMesh>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<PolygonMesh>::halfedge_descriptor halfedge_descriptor;
    typedef typename boost::graph_traits<PolygonMesh>::face_descriptor face_descriptor;
    typedef typename boost::graph_traits<PolygonMesh>::edge_descriptor edge_descriptor;

    typedef typename GeomTraits::Point_3  Point;
    typedef typename GeomTraits::Vector_3 Vector;
    typedef typename GeomTraits::Triangle_3 Triangle;
    typedef typename GeomTraits::FT FT;
    typedef std::vector<Triangle> Triangle_list;

    typedef CGAL::AABB_triangle_primitive<GeomTraits, typename Triangle_list::iterator> AABB_Primitive;
    typedef CGAL::AABB_traits<GeomTraits, AABB_Primitive> AABB_Traits;
    typedef CGAL::AABB_tree<AABB_Traits> Tree;

    // <for each halfedge around v, pair of incident halfedges to this halfedge around v>
    typedef std::pair<halfedge_descriptor, halfedge_descriptor> he_pair;
    typedef std::map<halfedge_descriptor, he_pair> Edges_around_map;

    //typedef typename CGAL::Monge_via_jet_fitting<GeomTraits> Monge_via_jet_fitting;
    //typedef typename Monge_via_jet_fitting::Monge_form Monge_form;


    /*
    typedef CGAL::internal::Cotangent_weight<
                    PolygonMesh,
                    VertexPointMap,
                    CGAL::internal::Cotangent_value_minimum_zero<PolygonMesh,
                                                                 VertexPointMap,
                                                                 CGAL::internal::Cotangent_value_Meyer_secure<PolygonMesh, VertexPointMap>
                                                                >
    >
    Weight_calculator_edge_based;


*/
    typedef CGAL::internal::Cotangent_value_Meyer_secure<PolygonMesh, VertexPointMap>
    Weight_calculator_angle_based;

    /*
    typedef CGAL::internal::Cotangent_value_clamped_2<PolygonMesh, VertexPointMap>
    Weight_calculator_clamped2;
*/

    typedef Cotangent_weight<PolygonMesh, VertexPointMap> Weight_calculator;

public:

    Curvature_flow(PolygonMesh& pmesh, VertexPointMap& vpmap, VertexConstraintMap& vcmap, EdgeConstraintMap& ecmap) :
        mesh_(pmesh), vpmap_(vpmap), vcmap_(vcmap), ecmap_(ecmap),
        cot_calculator_angle_based_(pmesh, vpmap),
        //cot_calculator_edge_based_(pmesh, vpmap),
        //my_weight_calculator_(pmesh, vpmap),
        //cot_clamped2_(pmesh, vpmap)
        weight_calculator_(pmesh, vpmap)

    {

/*
        std::size_t num_points = vertices(mesh_).size();
        std::size_t min_num_of_points = 6; // (d+1)(d+2)/2, for d=2
        if(num_points < min_num_of_points)
        {
            CGAL_error_msg("Find curvature: Not enough points in the mesh.");
        }
*/



    }

    template<typename FaceRange>
    void init_remeshing(const FaceRange& face_range)
    {
        check_vertex_range(face_range);

        check_constraints();

        BOOST_FOREACH(face_descriptor f, face_range)
        {
            input_triangles_.push_back(triangle(f));
        }

        tree_ptr_ = new Tree(input_triangles_.begin(), input_triangles_.end());
        tree_ptr_->accelerate_distance_queries();

        //mean_k_ = compute_mean_curvature();

    }

    std::size_t remove_degenerate_faces()
    {
        std::size_t nb_removed_faces = 0;

        // from repair.h
        nb_removed_faces = CGAL::Polygon_mesh_processing::remove_degenerate_faces(mesh_);

#ifdef CGAL_PMP_SMOOTHING_DEBUG
        std::cout<<"nb_collapsed_faces: "<<nb_removed_faces<<std::endl;
#endif

        return nb_removed_faces;
    }

    void curvature_smoothing()
    {
        std::map<vertex_descriptor, Point> barycenters;
        std::map<vertex_descriptor, Vector> n_map;


        BOOST_FOREACH(vertex_descriptor v, vrange)
        {
            if(!is_border(v, mesh_) && !is_constrained(v))
            {

                // normals
                Vector vn = compute_vertex_normal(v, mesh_,
                                                  Polygon_mesh_processing::parameters::vertex_point_map(vpmap_)
                                                  .geom_traits(traits_));
                n_map[v] = vn;


                // area around vertex
                double A = 0;
                //take one halfedge whose target is v
                BOOST_FOREACH(halfedge_descriptor ht, halfedges_around_target(v, mesh_))
                {
                    // is it ok if a face is degenerate?
                    A = area(faces_around_target(ht, mesh_), mesh_);
                    continue;
                }


                // find incident halfedges
                Edges_around_map he_map;
                typename Edges_around_map::iterator it;
                BOOST_FOREACH(halfedge_descriptor hi, halfedges_around_source(v, mesh_))
                    he_map[hi] = he_pair( next(hi, mesh_), prev(opposite(hi, mesh_), mesh_) );


                // calculate movement
                Vector curvature_normal = CGAL::NULL_VECTOR;
                Vector sum_of_vecs = CGAL::NULL_VECTOR;
                double sum_cot_weights = 0;
                for(it = he_map.begin(); it!= he_map.end(); ++it)
                {
                    halfedge_descriptor hi = it->first;
                    he_pair incd_edges = it->second;


                    //check_degeneracy(hi);

                    // weight
                    //double weight_angle_based = cot_angles(hi, incd_edges);
                    double weight_angle = weight_calculator_(hi, incd_edges);
                    //double weight_edge_based = cot_calculator_edge_based_(hi);
                    //double weight_my_weight = my_weight_calculator_(hi);

                    double weight_angle_based = cot_angles(hi, incd_edges);
                    //weight_edge_based = cot_calculator_edge_based_(hi);
                    //weight_my_weight = my_weight_calculator_(hi);

                    CGAL_assertion(weight_angle == weight_angle_based);

                    double weight = weight_angle; // testing
                    sum_cot_weights += weight;

                    /*
                    if(weight_edge_based > 1e-10)
                    {
                        CGAL_assertion(weight_angle_based > 2 * weight_edge_based - 1e-2 &&
                                       weight_angle_based < 2 * weight_edge_based + 1e-2);
                    }

                    if(weight_angle_based > 1e-10)
                    {
                        CGAL_assertion(weight_angle_based > 2 * weight_edge_based - 1e-2 &&
                                       weight_angle_based < 2 * weight_edge_based + 1e-2);
                    }
*/

                    // displacement vector
                    Point Xi = get(vpmap_, source(hi, mesh_));
                    Point Xj = get(vpmap_, target(hi, mesh_));
                    Vector vec(Xj, Xi); // towards outside
                    //Vector vec(Xi, Xj); // towards the inside

                    // add weight
                    vec *= weight;

                    // sum vecs
                    curvature_normal += vec;

                    //sum_of_vecs += vec; // just curious

                }

                // divide with total weight - if there is actually weight
                if(sum_cot_weights != 0)
                     curvature_normal /= sum_cot_weights;

                //Vector curvature_normal_k = vn * mean_k_ * 0.1;
                //curvature_normal = vn * sum_cot_weights;

                 //if(A != 0)
                 //    curvature_normal /= (4 * A);

                Point weighted_barycenter = get(vpmap_, v) - curvature_normal;
                //Point weighted_barycenter = get(vpmap_, v) + curvature_normal;
                barycenters[v] = weighted_barycenter;

            } // not on border
        } // all vertices

        typedef typename std::map<vertex_descriptor, Point>::value_type VP;

/*
        // compute locations on tangent plane
        typedef typename std::map<vertex_descriptor, Point>::value_type VP;
        std::map<vertex_descriptor, Point> new_locations;
        BOOST_FOREACH(const VP& vp, barycenters)
        {
            Point p = get(vpmap_, vp.first);
            Point q = vp.second;
            Vector n = n_map[vp.first];

            new_locations[vp.first] = q + ( n * Vector(q, p) ) * n ;
        }
*/


        // update location
        //BOOST_FOREACH(const VP& vp, new_locations) // uncomment for tangent plane
        BOOST_FOREACH(const VP& vp, barycenters)
            put(vpmap_, vp.first, vp.second);

    }


    void project_to_surface()
    {
        BOOST_FOREACH(vertex_descriptor v, vertices(mesh_))
        {
            if(!is_border(v, mesh_) ) // todo: && !is_constrained(v)
            {
                Point p_query = get(vpmap_, v);
                Point projected = tree_ptr_->closest_point(p_query);
                put(vpmap_, v, projected);
            }
        }
    }


private:
    Triangle triangle(face_descriptor f) const
    {
        halfedge_descriptor h = halfedge(f, mesh_);
        vertex_descriptor v1 = target(h, mesh_);
        vertex_descriptor v2 = target(next(h, mesh_), mesh_);
        vertex_descriptor v3 = target(next(next(h, mesh_), mesh_), mesh_);
        return Triangle(get(vpmap_, v1), get(vpmap_, v2), get(vpmap_, v3));
    }

    double sqlength(const vertex_descriptor& v1, const vertex_descriptor& v2) const
    {
        return to_double(CGAL::squared_distance(get(vpmap_, v1), get(vpmap_, v2)));
    }

    double sqlength(const halfedge_descriptor& h) const
    {
      vertex_descriptor v1 = target(h, mesh_);
      vertex_descriptor v2 = source(h, mesh_);
      return sqlength(v1, v2);
    }

    double sqlength(const edge_descriptor& e) const
    {
      return sqlength(halfedge(e, mesh_));
    }

    double cot_angles(const halfedge_descriptor& main_he, const he_pair& incd_edges)
    {
        vertex_descriptor vs = source(main_he, mesh_);
        vertex_descriptor vt = target(main_he, mesh_);
        vertex_descriptor v1 = target(incd_edges.first, mesh_);
        vertex_descriptor v2 = source(incd_edges.second, mesh_);

        CGAL_assertion(target(incd_edges.second, mesh_) == source(incd_edges.first, mesh_));

        Point p1 = get(vpmap_, v1);
        Point p2 = get(vpmap_, v2);
        Point pt = get(vpmap_, vt);
        Point ps = get(vpmap_, vs);

        // avoid degenerate cases
        Vector edge1(pt, p1);
        Vector edge2(pt, p2);
        Vector vec_main_he(pt, ps);
        double tolerance = 1e-3;
        /*
        if ( edge1.squared_length()           < tolerance ||
             edge2.squared_length()           < tolerance ||
             sqlength(main_he)                < tolerance ||
             (edge1 - vec_main_he).squared_length() < tolerance ||
             (edge2 - vec_main_he).squared_length() < tolerance   )
        {
            return 0;
            // zero means 90 degrees angle (also means no weight)
        }
        */

        //CGAL_assertion(vec_main_he.squared_length() > tolerance);

        double a1 = cot_calculator_angle_based_(vs, v1, vt);
        double a2 = cot_calculator_angle_based_(vs, v2, vt);

        //a1 = cot_clamped2_(vs, v1, vt);
        //a2 = cot_clamped2_(vs, v2, vt);

        return a1 + a2;
    }

    void check_degeneracy(halfedge_descriptor h1)
    {
        halfedge_descriptor h2 = next(h1, mesh_);
        halfedge_descriptor h3 = next(h2, mesh_);

        double a1 = get_angle(h1, h2);
        double a2 = get_angle(h2, h3);
        double a3 = get_angle(h3, h1);

        if(a1 < 0.05 || a2 < 0.05 || a3 < 0.05)
        {
            Euler::remove_face(h1, mesh_);
        }


    }

    double get_angle(halfedge_descriptor ha, halfedge_descriptor hb)
    {
        Vector a(get(vpmap_, source(ha, mesh_)), get(vpmap_, target(ha, mesh_)));
        Vector b(get(vpmap_, source(hb, mesh_)), get(vpmap_, target(hb, mesh_)));

        double angle = get_angle(a, b); // to fix

        return angle;
    }

    double get_angle(const Vector& e1, const Vector& e2)
    {
        //double rad_to_deg = 180. / CGAL_PI;
        double cos_angle = (e1 * e2)
          / std::sqrt(e1.squared_length() * e2.squared_length());

        return std::acos(cos_angle); //* rad_to_deg;
    }

    /*
    double compute_mean_curvature()
    {
        std::vector<Point> incident_points = gather_all_points();

        Monge_form monge_form;
        Monge_via_jet_fitting monge_fit;

        std::size_t d_fit = 2; // d_fit >= d_monge
        std::size_t d_monge = 2; // need 2 principal coeeficients
        std::size_t Nd = (d_fit + 1)*(d_fit + 1) / 2.0;
        CGAL_assertion(incident_points.size() >= Nd);

        monge_form = monge_fit(incident_points.begin(), incident_points.end(), d_fit, d_monge);
        const double k1 = monge_form.principal_curvatures(0);
        const double k2 = monge_form.principal_curvatures(1);

        return (k1 + k2) / 2.0;
    }


    std::vector<Point> points_around_vertex(vertex_descriptor v)
    {
        std::vector<Point> incident_vertices;
        for(halfedge_descriptor h : halfedges_around_target(v, mesh_))
        {
            vertex_descriptor vs = source(h, mesh_);
            incident_vertices.push_back(get(vpmap_, vs));
        }

        // temp assertion
        std::vector<Point> incident_vertices2;
        for(vertex_descriptor vi : vertices_around_target(v, mesh_))
        {
           incident_vertices2.push_back(get(vpmap_, vi));
        }
        CGAL_assertion(incident_vertices.size() == incident_vertices2.size());

        return incident_vertices;
    }

    std::vector<Point> gather_all_points()
    {
        //TODO SEE IF THERE IS SOMTEHTING alREADY FOR POINTS
        std::vector<Point> points;
        for(vertex_descriptor v : vertices(mesh_))
        {
            points.push_back(get(vpmap_, v)); // todo: preallocate it and fill it by pointing
        }

        return points;
    }
*/

    bool is_constrained(const edge_descriptor& e)
    {
        return get(ecmap_, e);
    }

    bool is_constrained(const vertex_descriptor& v)
    {
        return get(vcmap_, v);
    }

    void check_constraints()
    {
        BOOST_FOREACH(edge_descriptor e, edges(mesh_))
        {
            if (is_constrained(e))
            {
                vertex_descriptor vs = source(e, mesh_);
                vertex_descriptor vt = target(e, mesh_);
                put(vcmap_, vs, true);
                put(vcmap_, vt, true);
            }
        }
    }

    template<typename FaceRange>
    void check_vertex_range(const FaceRange& face_range)
    {
        BOOST_FOREACH(face_descriptor f, face_range)
        {
            BOOST_FOREACH(vertex_descriptor v, vertices_around_face(halfedge(f, mesh_), mesh_))
                vrange.insert(v);
        }
    }

    // data members
    PolygonMesh& mesh_;
    VertexPointMap& vpmap_;
    VertexConstraintMap vcmap_;
    EdgeConstraintMap ecmap_;
    Triangle_list input_triangles_;
    Tree* tree_ptr_;
    GeomTraits traits_;
    std::set<vertex_descriptor> vrange;

    // to fix
    //double min_sq_edge_len_;
    double mean_k_;
    // from Weights.h
    //CGAL::internal::Cotangent_value_Meyer_secure<PolygonMesh, VertexPointMap> cot_calculator_;
    //CGAL::internal::Cotangent_value_clamped_2<PolygonMesh, VertexPointMap> cot_calculator_;
    //CGAL::internal::Cotangent_value_clamped<PolygonMesh, VertexPointMap> cot_calculator_;

    /*
    typedef CGAL::internal::Cotangent_weight<
                    PolygonMesh,
                    typename boost::property_map<PolygonMesh, vertex_point_t>::type,
                    CGAL::internal::Cotangent_value_minimum_zero<PolygonMesh,
                                                                 typename boost::property_map<PolygonMesh, vertex_point_t>::type,
                                                                 CGAL::internal::Cotangent_value_Meyer_secure<PolygonMesh> > >     cot_calculator_;
                                                                 */
    //Weight_calculator_edge_based cot_calculator_edge_based_;
    Weight_calculator_angle_based cot_calculator_angle_based_;
    Weight_calculator weight_calculator_;
    //Weight_calculator_clamped2 cot_clamped2_;


};




} // internal
} // Polygon_mesh_processing
} // CGAL





#endif // CURVATURE_FLOW_IMPL_H
