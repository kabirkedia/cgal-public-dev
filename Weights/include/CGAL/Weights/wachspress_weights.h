// Copyright (c) 2020 GeometryFactory SARL (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
//
// Author(s)     : Dmitry Anisimov, David Bommes, Kai Hormann, Pierre Alliez
//

#ifndef CGAL_WACHSPRESS_WEIGHTS_H
#define CGAL_WACHSPRESS_WEIGHTS_H

// #include <CGAL/license/Weights.h>

// Internal includes.
#include <CGAL/Weights/internal/utils.h>
#include <CGAL/Weights/internal/polygon_utils.h>

namespace CGAL {
namespace Weights {

  /// \cond SKIP_IN_MANUAL
  namespace wachspress_ns {

    template<typename FT>
    const FT weight(
      const FT A1, const FT A2, const FT C) {

      FT w = FT(0);
      CGAL_precondition(A1 != FT(0) && A2 != FT(0));
      const FT prod = A1 * A2;
      if (prod != FT(0)) {
        const FT inv = FT(1) / prod;
        w = C * inv;
      }
      return w;
    }
  }
  /// \endcond

  #if defined(DOXYGEN_RUNNING)

  /*!
    \ingroup PkgWeightsRefWachspressWeights

    \brief computes the Wachspress weight in 2D at `q` using the points `p0`, `p1`,
    and `p2`, given a traits class `traits` with geometric objects, predicates, and constructions.
  */
  template<typename GeomTraits>
  const typename GeomTraits::FT wachspress_weight(
    const typename GeomTraits::Point_2& p0,
    const typename GeomTraits::Point_2& p1,
    const typename GeomTraits::Point_2& p2,
    const typename GeomTraits::Point_2& q,
    const GeomTraits& traits) { }

  /*!
    \ingroup PkgWeightsRefWachspressWeights

    \brief computes the Wachspress weight in 3D at `q` using the points `p0`, `p1`,
    and `p2`, given a traits class `traits` with geometric objects, predicates, and constructions.
  */
  template<typename GeomTraits>
  const typename GeomTraits::FT wachspress_weight(
    const typename GeomTraits::Point_3& p0,
    const typename GeomTraits::Point_3& p1,
    const typename GeomTraits::Point_3& p2,
    const typename GeomTraits::Point_3& q,
    const GeomTraits& traits) { }

  /*!
    \ingroup PkgWeightsRefWachspressWeights

    \brief computes the Wachspress weight in 2D at `q` using the points `p0`, `p1`,
    and `p2`, which are parameterized by a `Kernel` K.
  */
  template<typename K>
  const typename K::FT wachspress_weight(
    const CGAL::Point_2<K>& p0,
    const CGAL::Point_2<K>& p1,
    const CGAL::Point_2<K>& p2,
    const CGAL::Point_2<K>& q) { }

  /*!
    \ingroup PkgWeightsRefWachspressWeights

    \brief computes the Wachspress weight in 3D at `q` using the points `p0`, `p1`,
    and `p2`, which are parameterized by a `Kernel` K.
  */
  template<typename K>
  const typename K::FT wachspress_weight(
    const CGAL::Point_3<K>& p0,
    const CGAL::Point_3<K>& p1,
    const CGAL::Point_3<K>& p2,
    const CGAL::Point_3<K>& q) { }

  #endif // DOXYGEN_RUNNING

  /// \cond SKIP_IN_MANUAL
  template<typename GeomTraits>
  const typename GeomTraits::FT wachspress_weight(
    const typename GeomTraits::Point_2& t,
    const typename GeomTraits::Point_2& r,
    const typename GeomTraits::Point_2& p,
    const typename GeomTraits::Point_2& q,
    const GeomTraits& traits) {

    using FT = typename GeomTraits::FT;
    const FT A1 = internal::area_2(traits, r, q, t);
    const FT A2 = internal::area_2(traits, p, q, r);
    const FT C  = internal::area_2(traits, t, r, p);
    return wachspress_ns::weight(A1, A2, C);
  }

  template<typename GeomTraits>
  const typename GeomTraits::FT wachspress_weight(
    const CGAL::Point_2<GeomTraits>& t,
    const CGAL::Point_2<GeomTraits>& r,
    const CGAL::Point_2<GeomTraits>& p,
    const CGAL::Point_2<GeomTraits>& q) {

    const GeomTraits traits;
    return wachspress_weight(t, r, p, q, traits);
  }

  template<typename GeomTraits>
  const typename GeomTraits::FT wachspress_weight(
    const typename GeomTraits::Point_3& t,
    const typename GeomTraits::Point_3& r,
    const typename GeomTraits::Point_3& p,
    const typename GeomTraits::Point_3& q,
    const GeomTraits& traits) {

    using Point_2 = typename GeomTraits::Point_2;
    Point_2 tf, rf, pf, qf;
    internal::flatten(
      traits,
      t,  r,  p,  q,
      tf, rf, pf, qf);
    return wachspress_weight(tf, rf, pf, qf, traits);
  }

  template<typename GeomTraits>
  const typename GeomTraits::FT wachspress_weight(
    const CGAL::Point_3<GeomTraits>& t,
    const CGAL::Point_3<GeomTraits>& r,
    const CGAL::Point_3<GeomTraits>& p,
    const CGAL::Point_3<GeomTraits>& q) {

    const GeomTraits traits;
    return wachspress_weight(t, r, p, q, traits);
  }
  /// \endcond

  /*!
    \ingroup PkgWeightsRefBarycentricWachspressWeights

    \brief 2D Wachspress weights for polygons.

    This class implements 2D Wachspress weights ( \cite cgal:bc:fhk-gcbcocp-06,
    \cite cgal:bc:mlbd-gbcip-02, \cite cgal:bc:w-rfeb-75 ), which can be computed
    at any point inside a strictly convex polygon.

    Wachspress weights are well-defined and non-negative inside a strictly convex polygon.
    The weights are computed analytically using the formulation from the `wachspress_weight()`.

    \tparam VertexRange
    a model of `ConstRange` whose iterator type is `RandomAccessIterator`

    \tparam GeomTraits
    a model of `AnalyticWeightTraits_2`

    \tparam PointMap
    a model of `ReadablePropertyMap` whose key type is `VertexRange::value_type` and
    value type is `Point_2`. The default is `CGAL::Identity_property_map`.

    \cgalModels `BarycentricWeights_2`
  */
  template<
  typename VertexRange,
  typename GeomTraits,
  typename PointMap = CGAL::Identity_property_map<typename GeomTraits::Point_2> >
  class Wachspress_weights_2 {

  public:

    /// \name Types
    /// @{

    /// \cond SKIP_IN_MANUAL
    using Vertex_range = VertexRange;
    using Geom_traits = GeomTraits;
    using Point_map = PointMap;

    using Area_2 = typename GeomTraits::Compute_area_2;
    /// \endcond

    /// Number type.
    typedef typename GeomTraits::FT FT;

    /// Point type.
    typedef typename GeomTraits::Point_2 Point_2;

    /// @}

    /// \name Initialization
    /// @{

    /*!
      \brief initializes all internal data structures.

      This class implements the behavior of Wachspress weights
      for 2D query points inside strictly convex polygons.

      \param polygon
      an instance of `VertexRange` with the vertices of a strictly convex polygon

      \param traits
      a traits class with geometric objects, predicates, and constructions;
      the default initialization is provided

      \param point_map
      an instance of `PointMap` that maps a vertex from `polygon` to `Point_2`;
      the default initialization is provided

      \pre polygon.size() >= 3
      \pre polygon is simple
      \pre polygon is strictly convex
    */
    Wachspress_weights_2(
      const VertexRange& polygon,
      const GeomTraits traits = GeomTraits(),
      const PointMap point_map = PointMap()) :
    m_polygon(polygon),
    m_traits(traits),
    m_point_map(point_map),
    m_area_2(m_traits.compute_area_2_object()) {

      CGAL_precondition(
        polygon.size() >= 3);
      CGAL_precondition(
        internal::is_simple_2(polygon, traits, point_map));
      CGAL_precondition(
        internal::polygon_type_2(polygon, traits, point_map) ==
        internal::Polygon_type::STRICTLY_CONVEX);
      resize();
    }

    /// @}

    /// \name Access
    /// @{

    /*!
      \brief computes 2D Wachspress weights.

      This function fills a destination range with 2D Wachspress weights computed
      at the `query` point with respect to the vertices of the input polygon.

      The number of computed weights equals to the number of polygon vertices.

      \tparam OutIterator
      a model of `OutputIterator` whose value type is `FT`

      \param query
      a query point

      \param w_begin
      the beginning of the destination range with the computed weights

      \return an output iterator to the element in the destination range,
      one past the last weight stored
    */
    template<typename OutIterator>
    OutIterator operator()(
      const Point_2& query,
      OutIterator w_begin) {

      const bool normalize = false;
      return operator()(query, w_begin, normalize);
    }

    /// @}

    /// \cond SKIP_IN_MANUAL
    template<typename OutIterator>
    OutIterator operator()(
      const Point_2& query,
      OutIterator w_begin,
      const bool normalize) {

      return optimal_weights(
        query, w_begin, normalize);
    }
    /// \endcond

  private:

    // Fields.
    const VertexRange& m_polygon;
    const GeomTraits m_traits;
    const PointMap m_point_map;
    const Area_2 m_area_2;

    std::vector<FT> A;
    std::vector<FT> C;
    std::vector<FT> w;

    // Functions.
    void resize() {
      A.resize(m_polygon.size());
      C.resize(m_polygon.size());
      w.resize(m_polygon.size());
    }

    template<typename OutputIterator>
    OutputIterator optimal_weights(
      const Point_2& query,
      OutputIterator weights,
      const bool normalize) {

      // Get the number of vertices in the polygon.
      const std::size_t n = m_polygon.size();

      // Compute areas A and C following the area notation from [1].
      // Split the loop to make this computation faster.
      const auto& p1 = get(m_point_map, *(m_polygon.begin() + 0));
      const auto& p2 = get(m_point_map, *(m_polygon.begin() + 1));
      const auto& pn = get(m_point_map, *(m_polygon.begin() + (n - 1)));

      A[0] = m_area_2(p1, p2, query);
      C[0] = m_area_2(pn, p1, p2);

      for (std::size_t i = 1; i < n - 1; ++i) {
        const auto& pi0 = get(m_point_map, *(m_polygon.begin() + (i - 1)));
        const auto& pi1 = get(m_point_map, *(m_polygon.begin() + (i + 0)));
        const auto& pi2 = get(m_point_map, *(m_polygon.begin() + (i + 1)));

        A[i] = m_area_2(pi1, pi2, query);
        C[i] = m_area_2(pi0, pi1, pi2);
      }

      const auto& pm = get(m_point_map, *(m_polygon.begin() + (n - 2)));
      A[n - 1] = m_area_2(pn, p1, query);
      C[n - 1] = m_area_2(pm, pn, p1);

      // Compute unnormalized weights following the formula (28) from [1].
      CGAL_assertion(A[n - 1] != FT(0) && A[0] != FT(0));
      w[0] = C[0] / (A[n - 1] * A[0]);

      for (std::size_t i = 1; i < n - 1; ++i) {
        CGAL_assertion(A[i - 1] != FT(0) && A[i] != FT(0));
        w[i] = C[i] / (A[i - 1] * A[i]);
      }

      CGAL_assertion(A[n - 2] != FT(0) && A[n - 1] != FT(0));
      w[n - 1] = C[n - 1] / (A[n - 2] * A[n - 1]);

      // Normalize if necessary.
      if (normalize)
        internal::normalize(w);

      // Return weights.
      for (std::size_t i = 0; i < n; ++i)
        *(weights++) = w[i];
      return weights;
    }
  };

  /*!
    \ingroup PkgWeightsRefBarycentricWachspressWeights

    \brief computes 2D Wachspress weights for polygons.

    This function computes 2D Wachspress weights at a given `query` point
    with respect to the vertices of a strictly convex `polygon`, that is one
    weight per vertex. The weights are stored in a destination range
    beginning at `w_begin`.

    Internally, the class `Wachspress_weights_2` is used. If one wants to process
    multiple query points, it is better to use that class. When using the free function,
    internal memory is allocated for each query point, while when using the class,
    it is allocated only once, which is much more efficient. However, for a few query
    points, it is easier to use this function. It can also be used when the processing
    time is not a concern.

    \tparam PointRange
    a model of `ConstRange` whose iterator type is `RandomAccessIterator`
    and value type is `GeomTraits::Point_2`

    \tparam OutIterator
    a model of `OutputIterator` whose value type is `GeomTraits::FT`

    \tparam GeomTraits
    a model of `AnalyticWeightTraits_2`

    \param polygon
    an instance of `PointRange` with 2D points, which form a strictly convex polygon

    \param query
    a query point

    \param w_begin
    the beginning of the destination range with the computed weights

    \param traits
    a traits class with geometric objects, predicates, and constructions;
    this parameter can be omitted if the traits class can be deduced from the point type

    \return an output iterator to the element in the destination range,
    one past the last weight stored

    \pre polygon.size() >= 3
    \pre polygon is simple
    \pre polygon is strictly convex
  */
  template<
  typename PointRange,
  typename OutIterator,
  typename GeomTraits>
  OutIterator wachspress_weights_2(
    const PointRange& polygon,
    const typename GeomTraits::Point_2& query,
    OutIterator w_begin,
    const GeomTraits& traits) {

    Wachspress_weights_2<PointRange, GeomTraits> wachspress(
      polygon, traits);
    return wachspress(query, w_begin);
  }

  /// \cond SKIP_IN_MANUAL
  template<
  typename PointRange,
  typename Point_2,
  typename OutIterator>
  OutIterator wachspress_weights_2(
    const PointRange& polygon,
    const Point_2& query,
    OutIterator w_begin) {

    using GeomTraits = typename Kernel_traits<Point_2>::Kernel;
    const GeomTraits traits;
    return wachspress_weights_2(
      polygon, query, w_begin, traits);
  }
  /// \endcond

} // namespace Weights
} // namespace CGAL

#endif // CGAL_WACHSPRESS_WEIGHTS_H
