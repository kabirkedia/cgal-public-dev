// Copyright (c) 2010  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: $
// $Id: $
// 
//
// Author(s)     : Asaf Porat          <asafpor1@post.tau.ac.il>

#include <sstream>

#ifndef LINE_THROUGH_SEGMENTS_OUTPUT_OBJ_H
#define LINE_THROUGH_SEGMENTS_OUTPUT_OBJ_H

#include <CGAL/General_polygon_2.h>
#include <CGAL/Arrangement_with_history_2.h>
#include <CGAL/Arrangement_on_surface_with_history_2.h>
#include <CGAL/Arr_spherical_topology_traits_2.h>

#include <CGAL/Lines_through_segments_traits_3.h>
#include <CGAL/Lines_through_segments_point_adapt.h>
#include <CGAL/Lines_through_segments_general_functions.h>
#include <CGAL/Lines_through_segments_arr_ext_dcel.h>

/*************************************************************
 * This file contains different classes, each represents output object:
 *
 *************************************************************/

namespace CGAL {
  
/*************************************************************
 * The following class represents either a point, a curve, or a general
 * polygon on the arrangement
 *    This mapped element is parametrized by the variables S1_t and S2_t.
 *    S1_t and S2_t are points on S1 S2 and specific values of each represents
 *    a line in 3D.
 *
 *************************************************************/

template <typename Traits_3_>
class Lines_through_segments_mapped_2 {
public:
   typedef Traits_3_                                    Traits_3;
protected:
  typedef typename Traits_3::Traits_arr_on_plane_2      Traits_2;
  typedef typename Traits_3::Rational_kernel            Rational_kernel;
  typedef typename Rational_kernel::Segment_3           Rational_segment_3;
  typedef typename Rational_kernel::Point_3             Rational_point_3;
   typedef typename Rational_kernel::Line_3             Rational_line_3;

  typedef typename Traits_3::Algebraic_NT               Algebraic;

public:
  typedef Traits_3
    Lines_through_segments_traits_3;
  typedef typename Traits_3::Alg_kernel::Line_3         Mapped_line_3;

  typedef typename Traits_3::Alg_kernel::Point_3        Alg_point_3;
   
//  typedef typename Rational_kernel::Point_2             Rational_point_2;
  typedef typename Traits_2::X_monotone_curve_2         X_monotone_curve_2;
  typedef typename Traits_3::Traits_arr_on_plane_2::Point_2 Point_2;
  typedef CGAL::General_polygon_2<Traits_2>             General_polygon_2;

   typedef boost::variant<Point_2, X_monotone_curve_2, General_polygon_2// ,
                          // Rational_point_2
                          >
    Mapped_transversal;
      
private:
      
  Mapped_transversal m_mapped_transversal;
  const Rational_segment_3* m_s1;
  const Rational_segment_3* m_s2;     

public:
  Lines_through_segments_mapped_2(const Lines_through_segments_mapped_2& mapped)
  {
    m_mapped_transversal = mapped.m_mapped_transversal;
    m_s1 = mapped.m_s1;
    m_s2 = mapped.m_s2;
  }

  Lines_through_segments_mapped_2(const Point_2& p, 
                                  const Rational_segment_3& s1,
                                  const Rational_segment_3& s2)
  {
    m_mapped_transversal = p;
    m_s1 = &s1;
    m_s2 = &s2;
  }

  Lines_through_segments_mapped_2(const X_monotone_curve_2& curve, 
                                  const Rational_segment_3& s1,
                                  const Rational_segment_3& s2)
  {
    m_mapped_transversal = curve;
    m_s1 = &s1;
    m_s2 = &s2;
  }
      
  template <typename Input_iterator>
  Lines_through_segments_mapped_2(Input_iterator curves_begin,
                                  Input_iterator curves_end, 
                                  const Rational_segment_3& s1,
                                  const Rational_segment_3& s2)
  {
    General_polygon_2 polygon;
    m_s1 = &s1;
    m_s2 = &s2;
    while(curves_begin != curves_end)
    {
      polygon.push_back(*curves_begin);
      curves_begin++;
    }
    m_mapped_transversal = polygon;
  }

  Mapped_transversal mapped_transversal()
  {
    return m_mapped_transversal;
  }
      
public:   
  /* Obtain the line represented by a given planar point. */
  Mapped_line_3 line(const Point_2& p)
  {
    typedef Lines_through_segments_point_adapt_2<Traits_3,
                                                 typename Traits_2::Point_2,
                                                 Algebraic> Point_2;
    typedef Lines_through_segments_general_functions<Traits_3> LTS_g_func;
    LTS_g_func m_g_func;

    Lines_through_segments_get_algebraic_number_adapt<Traits_3>
      get_algebraic_number_adapt;

    Mapped_line_3 common_line;
         
    Point_2 int_adapt(p);
    Algebraic S1_t = get_algebraic_number_adapt(int_adapt.x());
    Algebraic S2_t = get_algebraic_number_adapt(int_adapt.y());

    int status = 
      m_g_func.get_line_from_intersection_point(S1_t, S2_t, *m_s1, *m_s2,
                                                common_line);

    /* The query will fail in case S1 intersect S2
       and the line passes through the intersection point,
       in such case return a line through the point and (0,0,0)
       or (1,1,1). */
    if (status != LTS_g_func::CGAL_QUERY_SUCCEED)
    {
      Rational_point_3 intersection_point_S1S2;
      Rational_kernel rational_kernel;
      CGAL::Object result =
        rational_kernel.intersect_3_object()(m_s1->supporting_line(),
                                             m_s2->supporting_line());
      if (CGAL::assign(intersection_point_S1S2, result))
      {
        if (intersection_point_S1S2 != Rational_point_3(0,0,0))
        {
          Alg_point_3 temp(intersection_point_S1S2.x(),
                           intersection_point_S1S2.y(),
                           intersection_point_S1S2.z());
                 
          common_line = Mapped_line_3(Alg_point_3(0,0,0), temp);
        }
        else
        {
          Alg_point_3 temp(intersection_point_S1S2.x(),
                           intersection_point_S1S2.y(),
                           intersection_point_S1S2.z());
                  
          common_line = Mapped_line_3(Alg_point_3(1,1,1), temp);
        }
      }
      else
      {
        CGAL_error_msg("Error - a line passes through the intersection point of S1,S2");
      }
    }
         
    return common_line;
  }

  // Rational_line_3 rational_line()
  // {
  //    Rational_point_2* rpoint_obj;
  //    if ((rpoint_obj = boost::get<Rational_point_2>(&m_mapped_transversal)))
  //    {
  //       Rational_line_3 common_line;
  //       typedef Lines_through_segments_general_functions<Traits_3> LTS_g_func;
  //       LTS_g_func m_g_func;

  //       CGAL_assertion_code(int status =)
  //          m_g_func.get_line_from_intersection_point(rpoint_obj->x(), 
  //                                                    rpoint_obj->y(), 
  //                                                    *m_s1, *m_s2,
  //                                                    common_line);
  //       CGAL_assertion(status == LTS_g_func::CGAL_QUERY_SUCCEED);
  //       return common_line;
  //    }
  //    CGAL_error_msg("The line is not rational");
  // }
   
  /* Obtain a representative line. */
  Mapped_line_3 line()
  {
    return line(boost::apply_visitor(Mapped_point(), m_mapped_transversal));
  }
           
  virtual std::string to_string() const
  {
    std::ostringstream o;
    o << "MAPPED_2" << std::endl;
    o << "S1 = " << *m_s1 << std::endl;
    o << "S2 = " << *m_s2 << std::endl;         
    o << boost::apply_visitor(Mapped_to_string(), m_mapped_transversal)
      << std::endl;
    return o.str();
  }

private:
  class Mapped_to_string : public boost::static_visitor<std::string>
  {
  public:
    // std::string operator()(const Rational_point_2& to_print) const
    // {
    //   std::ostringstream o;
    //   o << to_print << std::endl;
    //   return o.str();
    // }

    std::string operator()(const Point_2& to_print) const
    {
      std::ostringstream o;
#if USE_CONIC_TRAITS
      o << to_print << std::endl;
#endif
      return o.str();
    }

    std::string operator()(const X_monotone_curve_2& to_print) const
    {
      std::ostringstream o;
      o << to_print << std::endl;
      return o.str();
    }

    std::string operator()(const General_polygon_2& to_print) const
    {
      std::ostringstream o;
      o << to_print << std::endl;
      return o.str();
    }
  };


private:
  class Mapped_point : public boost::static_visitor<Point_2>
  {
  public:
         
    // Point_2 operator()(const Rational_point_2& point) const
    // {
    //    CGAL_error_msg("For rational line use rational_line()");
    // }

    Point_2 operator()(const Point_2& point) const
    {
      return point;
    }

    Point_2 operator()(const X_monotone_curve_2& curve) const
    {
      return curve.source();
    }

    Point_2 operator()(const General_polygon_2& polygon) const
    {
      return (polygon.curves_begin())->source();
    }
  };
};

   template <typename Traits_3, typename Ext_obj>
class Lines_through_segments_mapped_2_with_arrangement :
      public Lines_through_segments_mapped_2<Traits_3>
{
   typedef Lines_through_segments_mapped_2<Traits_3> Base;
public:
   /***************************************************/
   /*    Arrangement on plane typedefs.               */
   /***************************************************/
   typedef typename Traits_3::Traits_arr_on_plane_2
   Traits_arr_on_plane_2;
   
   /* Extended each edge with its creator line segment.*/
   typedef Lines_through_segments_arr_ext_dcel<Traits_arr_on_plane_2,
                                               Ext_obj>
   Dcel_on_plane;
   typedef CGAL::Arrangement_with_history_2<Traits_arr_on_plane_2,
                                            Dcel_on_plane>  
   Arrangement_2;
   
private:
   const Arrangement_2* m_arr;

public:
   Lines_through_segments_mapped_2_with_arrangement(const Base& mapped) :
     Base(mapped)
   {
      CGAL_error_msg("Unexpected error");
   }

   Lines_through_segments_mapped_2_with_arrangement
   (const Lines_through_segments_mapped_2_with_arrangement& to_copy)
      : Base(to_copy)
   {
      m_arr = to_copy.m_arr;
   }
   
  Lines_through_segments_mapped_2_with_arrangement
  (const typename Base::X_monotone_curve_2& curve, 
   const typename Base::Rational_segment_3& s1,
   const typename Base::Rational_segment_3& s2) :
    Base(curve,s1,s2)
  {
     m_arr = NULL;
  }

  Lines_through_segments_mapped_2_with_arrangement
  (const typename Base::Point_2& point, 
   const typename Base::Rational_segment_3& s1,
   const typename Base::Rational_segment_3& s2) :
    Base(point,s1,s2)
  {
     m_arr = NULL;
  }

  // Lines_through_segments_mapped_2_with_arrangement
  // (// const typename Base::Rational_point_2& point, 
  //  const typename Base::Rational_segment_3& s1,
  //  const typename Base::Rational_segment_3& s2) :
  //   Base(point,s1,s2)
  // {
  //    m_arr = NULL;
  // }

   template <typename Input_iterator>
   Lines_through_segments_mapped_2_with_arrangement
   (Input_iterator curves_begin,
    Input_iterator curves_end, 
    const typename Base::Rational_segment_3& s1,
    const typename Base::Rational_segment_3& s2) :
     Base(curves_begin, curves_end, s1,s2)
   {
      m_arr = NULL;
   }

   void set_arrangement(const Arrangement_2* arr)
   {
      m_arr = arr;
   }

   const Arrangement_2* arrangement()
   {
      return m_arr;
   }
   
  std::string to_string() const
   {
      std::ostringstream o;
      o << Base::to_string();
      
      o << "Arrangement size:" 
        << "   V = " << m_arr->number_of_vertices()
        << ",  E = " << m_arr->number_of_edges()
        << ",  F = " << m_arr->number_of_faces() << std::endl;

      return o.str();
  }
};
   
template <typename Traits_3>
class Lines_through_segments_through_3 {
  typedef typename Traits_3::Rational_kernel          Rational_kernel;
      
public:
  typedef Traits_3
  Lines_through_segments_traits_3;
  typedef typename Rational_kernel::Point_3           Point_3;
  typedef typename Rational_kernel::Segment_3         Segment_3;
  typedef std::pair<Point_3, Segment_3>               Point_3_segment_3;
      
  typedef boost::variant<Point_3, Segment_3, std::pair<Point_3, Segment_3> >
    Through_transversal;
      
private:
  Through_transversal m_through_transversal;
      
public:
  Lines_through_segments_through_3(const Lines_through_segments_through_3& through)
  {
    m_through_transversal = through.m_through_transversal;
  }
      
  Lines_through_segments_through_3(const Point_3& point)
  {
    m_through_transversal = point;
  }

  Lines_through_segments_through_3(const Segment_3& segment)
  {
    m_through_transversal = segment;
  }

  Lines_through_segments_through_3(const Segment_3& segment,
                                   const Point_3& point)
  {
    m_through_transversal = std::make_pair(point,segment);
  }   

  Through_transversal through_transversal()
  {
    return m_through_transversal;
  }

  virtual std::string to_string() const
  {
    std::ostringstream o;
    o << "THROUGH_3" << std::endl;
    o << boost::apply_visitor(Through_to_string(), m_through_transversal)
      << std::endl;
    return o.str();
  }
      
private:
  class Through_to_string :
    public boost::static_visitor<std::string>
  {
  public:
    std::string operator()(const Point_3& to_print) const
    {
      std::ostringstream o;
      o << to_print << std::endl;
      return o.str();
    }

    std::string operator()(const Segment_3& to_print) const
    {
      std::ostringstream o;
      o << to_print << std::endl;
      return o.str();
    }

    std::string operator()(const Point_3_segment_3& to_print) const
    {
      std::ostringstream o;
      o << to_print.first << std::endl;
      o << to_print.second << std::endl;
      return o.str();
    }
  };
};

   template <typename Traits_3, typename Ext_obj>
class Lines_through_segments_through_3_with_arrangement :
      public Lines_through_segments_through_3<Traits_3>
{
   typedef Lines_through_segments_through_3<Traits_3> Base;
public:
   /***************************************************/
   /*    Arrangement on sphere typedefs.              */
   /***************************************************/
   typedef typename Traits_3::Traits_arr_on_sphere_2
   Traits_arr_on_sphere_2;
   
   typedef Lines_through_segments_arr_ext_dcel<Traits_arr_on_sphere_2,
                                               Ext_obj>
   Dcel_geom_traits;
   typedef CGAL::Arr_spherical_topology_traits_2<Traits_arr_on_sphere_2,
                                                 Dcel_geom_traits>
   Topol_traits_2;
   
   typedef CGAL::Arrangement_on_surface_with_history_2<Traits_arr_on_sphere_2,
                                                       Topol_traits_2>
   Arrangement_2;
   
private:
   const Arrangement_2* m_arr;

public:
   Lines_through_segments_through_3_with_arrangement(const Base& through) :
     Base(through)
   {
      CGAL_error_msg("Unexpected error");
   }

  Lines_through_segments_through_3_with_arrangement
  (const Lines_through_segments_through_3_with_arrangement& through) :
    Base(through)
  {
     m_arr = through.m_arr;
  }
      
   Lines_through_segments_through_3_with_arrangement
   (const typename Base::Point_3& point) :
     Base(point)
  {
     m_arr = NULL;
  }

   Lines_through_segments_through_3_with_arrangement
   (const typename Base::Segment_3& segment) :
     Base(segment)
  {
     m_arr = NULL;
  }

  Lines_through_segments_through_3_with_arrangement
  (const typename Base::Segment_3& segment,
   const typename Base::Point_3& point) :
    Base(segment, point)
  {
     m_arr = NULL;
  }   

   void set_arrangement(const Arrangement_2* arr)
   {
      m_arr = arr;
   }

   const Arrangement_2* arrangement()
   {
      return m_arr;
   }
   
  std::string to_string() const
   {
      std::ostringstream o;
      o << Base::to_string();
      /* m_arr equals null in case of through segment. */
      if (m_arr != NULL)
      {
         o << "Arrangement size:" 
           << "   V = " << m_arr->number_of_vertices()
           << ",  E = " << m_arr->number_of_edges()
           << ",  F = " << m_arr->number_of_faces() << std::endl;
      }
      
      return o.str();
  }
      

};

// template <typename Lines_through_segments_traits_3_,
//           typename Transversal_without_segments>
// class Lines_through_segments_transversal_with_segments
// {
//    typedef typename 
//    Lines_through_segments_traits_3_::Rational_kernel::Segment_3 
//    Rational_segment_3;
//    typedef typename 
//    Lines_through_segments_traits_3_::Rational_kernel::Line_3 Line_3;   

// private:
//    const Rational_segment_3* m_s1;
//    const Rational_segment_3* m_s2;
//    const Rational_segment_3* m_s3;
//    const Rational_segment_3* m_s4;
//    Transversal_without_segments m_tws;
         
// public:   
//    Lines_through_segments_transversal_with_segments()
//    {
//       m_s1 = NULL;
//       m_s2 = NULL;
//       m_s3 = NULL;
//       m_s4 = NULL;
//    }
         
//    Lines_through_segments_transversal_with_segments(const Rational_segment_3 *s1,
//                                                     const Rational_segment_3 *s2,
//                                                     const Rational_segment_3 *s3,
//                                                     const Rational_segment_3 *s4,
//                                                     const Transversal_without_segments& tws)
//    {
//       m_s1 = s1;
//       m_s2 = s2;
//       m_s3 = s3;
//       m_s4 = s4;
//       m_tws = tws;
//    }
         
//    Lines_through_segments_transversal_with_segments(
//       const Lines_through_segments_transversal_with_segments &obj)
//    {
//       m_s1 = obj.m_s1;
//       m_s2 = obj.m_s2;
//       m_s3 = obj.m_s3;
//       m_s4 = obj.m_s4;
//       m_tws = obj.m_tws;
//    }

//    Transversal_without_segments transversal() const
//    {
//       return m_tws;
//    }

//    const Rational_segment_3* s1() const
//    {
//       return m_s1;
//    }

//    const Rational_segment_3* s2() const
//    {
//       return m_s2;
//    }

//    const Rational_segment_3* s3() const
//    {
//       return m_s3;
//    }
         
//    const Rational_segment_3* s4() const
//    {
//       return m_s4;
//    }
         
//    std::string to_string() const
//    {
//       std::ostringstream o;
//       o << "S1 = " << *m_s1 << std::endl;
//       o << "S2 = " << *m_s2 << std::endl;
//       o << "S3 = " << *m_s3 << std::endl;
//       o << "S4 = " << *m_s4 << std::endl;
//       o << m_tws << std::endl;
            
//       return o.str();
//    }
// };

   template <typename Traits_3, typename Ext_obj>
class Lines_through_segments_output_obj {
  typedef typename Traits_3::Rational_kernel::Segment_3    Rational_segment_3;
  typedef typename Traits_3::Rational_kernel::Line_3       Line_3;

public:      
  typedef Traits_3
    Lines_through_segments_traits_3;
  typedef Lines_through_segments_mapped_2<Traits_3>        Mapped_2;
  typedef Lines_through_segments_through_3<Traits_3>       Through_3;
  typedef Lines_through_segments_mapped_2_with_arrangement<Traits_3, Ext_obj> 
  Mapped_2_with_arr;
      typedef Lines_through_segments_through_3_with_arrangement<Traits_3, Ext_obj>
  Through_3_with_arr;

  typedef typename Mapped_2::Mapped_transversal            Mapped_transversal;
  typedef typename Through_3::Through_transversal          Through_transversal;
      
   typedef boost::variant<Line_3, Through_3, Mapped_2 >     Transversal;
   typedef boost::variant<Line_3, 
                          Through_3_with_arr, 
                          Mapped_2_with_arr >     Transversal_with_arr;
      
   typedef CGAL::cpp0x::array<const Rational_segment_3*, 4> Segments;
   typedef std::pair<Transversal, Segments >
   Transversal_with_segments;
   typedef std::pair<Transversal_with_arr, Segments >
   Transversal_with_segments_with_arr;

};

} //namespace CGAL

#endif /* LINE_THROUGH_SEGMENTS_OUTPUT_OBJ_H */
