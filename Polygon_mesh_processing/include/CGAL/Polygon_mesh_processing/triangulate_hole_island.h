// Copyright (c) 2018 GeometryFactory (France).
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
// Author(s)     : Konstantinos Katrioplas

#ifndef CGAL_PMP_TRIANGULATE_HOLE_ISLAND_H
#define CGAL_PMP_TRIANGULATE_HOLE_ISLAND_H


#include <CGAL/Polygon_mesh_processing/internal/Hole_filling/island_triangulate_hole_polyline.h>
#include <CGAL/Polygon_mesh_processing/internal/Hole_filling/Triangulate_hole_polyline.h>


namespace CGAL {
namespace Polygon_mesh_processing{


template<typename PointRange>
using Domain = CGAL::internal::Domain<PointRange>;


template <typename PointRange, typename PolygonMesh>
std::size_t
triangulate_hole_islands(const PointRange& boundary,
                         const std::vector<PointRange>& islands,
                               PolygonMesh& mesh)
{
  // create domain from the boundary
  std::size_t boundary_size = boundary.size();
  if( *boost::begin(boundary) == *cpp11::prev(boost::end(boundary)) )
    --boundary_size;

  // indices
  std::vector<int> b_indices(boundary_size);
  for(std::size_t i = 0; i < boundary_size; ++i)
    b_indices[i] = i;

  Domain<PointRange> domain(b_indices);

  std::size_t number_of_vertices_on_islands = 0;
  // add islands if there is one
  if(!islands.empty())
  {
    std::size_t ind = b_indices.size(); // start
    for(auto island : islands)
    {
      std::vector<int> h_ids;
      std::size_t island_size = island.size();
      // dont use the last one
      if( *boost::begin(island) == *cpp11::prev(boost::end(island)) )
        --island_size;

      number_of_vertices_on_islands += island_size;

      h_ids.reserve(island_size);
      // generate their ids
      for(std::size_t j = 0; j < island_size; ++j)
      {
        h_ids.push_back(ind);
        ++ind;
      }

      // push it to the domain
      domain.add_hole(h_ids);

    }
  }

  // assign access edge on the boundary
  int i =  0;
  int k = static_cast<int>(b_indices.size())-1;;
  std::size_t count = 0;

  // weight calculator
  typedef CGAL::internal::Weight_min_max_dihedral_and_area      Weight;
  typedef CGAL::internal::Weight_calculator<Weight,
                CGAL::internal::Is_not_degenerate_triangle>  WC;

  // lookup tables
  typedef CGAL::internal::Lookup_table_map<Weight> WeightTable;
  typedef CGAL::internal::Lookup_table_map<int> LambdaTable;
  int n = static_cast<int>(b_indices.size() + number_of_vertices_on_islands);
  WeightTable W(n, Weight::DEFAULT());
  LambdaTable lambda(n, -1);

  // put together points list
  PointRange points;
  points.reserve(boundary_size + number_of_vertices_on_islands);
  points.insert(points.end(), boundary.begin(), boundary.end() - 1); // boundary has not been reduced
  if(!islands.empty())
  {
    for(auto island : islands)
    {
      points.insert(points.end(), island.begin(), island.end() - 1);
    }
  }

  // output triangulation
  std::vector<std::vector<std::size_t> > triplets;

  CGAL::internal::Triangulate_hole_with_islands<PointRange, WC, WeightTable, LambdaTable>
      triangulation(domain, points, W, lambda, WC());
  triangulation.do_triangulation(i, k, triplets, count);

  // with process_domain_extra the output collected already
  //triangulation.collect_triangles(triplets, i, k); // start from the initial access edge
  triangulation.visualize(points, triplets, mesh);

  return count;
}



template <typename PointRange, typename PolygonMesh>
std::size_t
triangulate_hole_islands(const PointRange& boundary,
                         const PointRange& hole,
                               PolygonMesh& mesh)
{
  // create domain from the boundary
  std::size_t boundary_size = boundary.size();
  if( *boost::begin(boundary) == *cpp11::prev(boost::end(boundary)) )
    --boundary_size;

  // indices
  std::vector<int> b_indices(boundary_size);
  for(std::size_t i = 0; i < boundary_size; ++i)
    b_indices[i] = i;

  Domain<PointRange> domain(b_indices);

  // add hole if there is one
  std::size_t hole_size = hole.size();
  std::vector<int> h_ids;
  if(!hole.empty())
  {
    if( *boost::begin(hole) == *cpp11::prev(boost::end(hole)) )
      --hole_size;
    h_ids.reserve(hole_size);
    std::size_t n_b =  b_indices.size();
    for(std::size_t i = n_b; i < n_b + hole_size; ++i)
      h_ids.push_back(i);
    domain.add_hole(h_ids);
  }

  // assign access edge on the boundary
  int i =  0;
  int k = static_cast<int>(b_indices.size())-1;;
  std::size_t count = 0;

  // weight calculator
  typedef CGAL::internal::Weight_min_max_dihedral_and_area      Weight;
  typedef CGAL::internal::Weight_calculator<Weight,
                CGAL::internal::Is_not_degenerate_triangle>  WC;

  // lookup tables
  typedef CGAL::internal::Lookup_table_map<Weight> WeightTable;
  typedef CGAL::internal::Lookup_table_map<int> LambdaTable;
  int n = static_cast<int>(b_indices.size() + h_ids.size()); // todo: function to return their sum
  WeightTable W(n, Weight::DEFAULT());
  LambdaTable lambda(n, -1);

  // put together points list
  PointRange points;
  points.reserve(boundary_size + hole_size);
  points.insert(points.end(), boundary.begin(), boundary.end() - 1); // boundary has not been reduced
  if(!hole.empty())
  {
    points.insert(points.end(), hole.begin(), hole.end() - 1);
  }

  // output triangulation
  std::vector<std::vector<std::size_t> > triplets;

  CGAL::internal::Triangulate_hole_with_islands<PointRange, WC, WeightTable, LambdaTable>
      triangulation(domain, points, W, lambda, WC());
  triangulation.do_triangulation(i, k, triplets, count);

  // with process_domain_extra the output collected already
  //triangulation.collect_triangles(triplets, i, k); // start from the initial access edge
  triangulation.visualize(points, triplets, mesh);

  return count;
}





} //namespace Polygon_mesh_processing
} //namespace CGAL

#endif // CGAL_PMP_TRIANGULATE_HOLE_ISLAND_H