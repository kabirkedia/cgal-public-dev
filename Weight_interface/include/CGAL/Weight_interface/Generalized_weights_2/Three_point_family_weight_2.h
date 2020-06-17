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
// Author(s)     : Dmitry Anisimov
//

#ifndef CGAL_GENERALIZED_THREE_POINT_FAMILY_WEIGHT_2_H
#define CGAL_GENERALIZED_THREE_POINT_FAMILY_WEIGHT_2_H

// #include <CGAL/license/Weight_interface.h>

// Internal includes.
#include <CGAL/Weight_interface/internal/utils_2.h>

namespace CGAL {
namespace Generalized_weights {

  /*!
    \ingroup PkgWeightInterfaceRef2DWeights

    \brief 2D three point family weight.

    The full weight is computed as

    \f$w = \frac{r_p^a A_m - r^a B + r_m^a A}{A_m A}\f$

    with notations shown in the figure below and \f$a\f$ any real number
    being the power parameter.

    For \f$a = 0\f$ this weight is equal to the
    `CGAL::Generalized_weights::Wachspress_weight_2` and
    `CGAL::Generalized_weights::Authalic_weight_2`.

    For \f$a = 1\f$ this weight is equal to the
    `CGAL::Generalized_weights::Mean_value_weight_2` and
    `CGAL::Generalized_weights::Tangent_weight_2`.

    For \f$a = 2\f$ this weight is equal to the
    `CGAL::Generalized_weights::Discrete_harmonic_weight_2` and
    `CGAL::Generalized_weights::Cotangent_weight_2`.

    \cgalFigureBegin{three_point_family_weight, three_point_family.svg}
      Notation used for the three point family weight.
    \cgalFigureEnd

    \tparam GeomTraits
    must be a model of `AnalyticTraits_2`.

    \cgalModels `AnalyticWeight_2`
  */
  template<typename GeomTraits>
  class Three_point_family_weight_2 {

  public:

    /// \name Types
    /// @{

    /// \cond SKIP_IN_MANUAL
    using GT = GeomTraits;
    /// \endcond

    /// Number type.
    typedef typename GeomTraits::FT FT;

    /// 2D point type.
    typedef typename GeomTraits::Point_2 Point_2;

    /// 3D point type.
    typedef typename GeomTraits::Point_3 Point_3;

    /// @}

    /// \name Initialization
    /// @{

    /*!
      \brief initializes all internal data structures.

      \param a
      the power parameter.

      \param traits
      An instance of `GeomTraits`. The default initialization is provided.
    */
    Three_point_family_weight_2(
      const FT a = FT(1), // default is for mean value coordinates
      const GeomTraits traits = GeomTraits()) :
    m_p(a), m_traits(traits)
    { }

    /// @}

    /// \name Access
    /// @{

    /*!
      \brief computes 2D three point family weight.
    */
    const FT operator()(
      const Point_2& query,
      const Point_2& vm,
      const Point_2& vj,
      const Point_2& vp) const {

      return weight_2(query, vm, vj, vp);
    }

    /*!
      \brief computes 2D three point family weight.
    */
    const FT operator()(
      const Point_3& query,
      const Point_3& vm,
      const Point_3& vj,
      const Point_3& vp) const {

      return weight_3(query, vm, vj, vp);
    }

    /// @}

  private:
    const FT m_p;
    const GeomTraits m_traits;

    const FT weight_2(
      const Point_2& query,
      const Point_2& vm,
      const Point_2& vj,
      const Point_2& vp) const {

      const FT rm = internal::distance_2(m_traits, query, vm);
      const FT rj = internal::distance_2(m_traits, query, vj);
      const FT rp = internal::distance_2(m_traits, query, vp);

      const auto area_2 =
        m_traits.compute_area_2_object();
      const FT Am = area_2(vm, vj, query);
      const FT Aj = area_2(vj, vp, query);
      const FT Bj = area_2(vm, vp, query);

      return weight(
        rm, rj, rp, Am, Aj, Bj);
    }

    const FT weight_3(
      const Point_3& query,
      const Point_3& vm,
      const Point_3& vj,
      const Point_3& vp) const {

      Point_2 pq, pm, pj, pp;
      internal::flatten(
        m_traits, query, vm, vj, vp,
        pq, pm, pj, pp);
      return weight_2(pq, pm, pj, pp);
    }

    const FT weight(
      const FT rm, const FT rj, const FT rp,
      const FT Am, const FT Aj, const FT Bj) const {

      FT w = FT(0);
      CGAL_assertion(Am != FT(0) && Aj != FT(0));
      const FT prod = Am * Aj;
      if (prod != FT(0)) {
        const FT inv = FT(1) / prod;
        FT a = rm;
        FT b = rj;
        FT c = rp;
        if (m_p != FT(1)) {
          a = internal::power(m_traits, rm, m_p);
          b = internal::power(m_traits, rj, m_p);
          c = internal::power(m_traits, rp, m_p);
        }
        w = (a * Am - b * Bj + c * Aj) * inv;
      }
      return w;
    }
  };

} // namespace Generalized_weights
} // namespace CGAL

#endif // CGAL_GENERALIZED_THREE_POINT_FAMILY_WEIGHT_2_H
