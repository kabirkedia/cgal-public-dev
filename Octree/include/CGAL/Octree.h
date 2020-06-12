// Copyright (c) 2007-2008  INRIA (France).
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
// Author(s)     : Tong Zhao, Cédric Portaneri

#ifndef CGAL_OCTREE_3_H
#define CGAL_OCTREE_3_H

/*
 * Not present or relevant for benchmarking
 */
//#include <CGAL/license/Implicit_surface_reconstruction_3.h>

#include <CGAL/Octree/Octree_node.h>

#include <CGAL/bounding_box.h>
#include <boost/iterator/transform_iterator.hpp>
#include <CGAL/Aff_transformation_3.h>
#include <CGAL/aff_transformation_tags.h>

#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Search_traits_adapter.h>

/*
 * These headers were not included here originally
 * Adding them was necessary to make this header self sufficient
 */
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <fstream>

#include <stack>
#include <queue>
#include <vector>
#include <math.h>

namespace CGAL {

  struct HashIntPoint_3 {
    uint64_t operator()(const IntPoint_3 &pt) const {
      return ((std::hash<uint64_t>()(static_cast<uint64_t>(pt.x())) ^
               (std::hash<uint64_t>()(static_cast<uint64_t>(pt.y())) << 1)) >> 1)
             ^ (std::hash<uint64_t>()(static_cast<uint64_t>(pt.z())) << 1);
    }
  };

  enum DebugOctreeVisuType {
    SHOW_ALL_LEAFS = 0,
    SHOW_NON_EMPTY_LEAFS = 1,
    SHOW_NON_EMPTY_NODES = 2
  };

  template<class Kernel,
          class PointRange,
          class PointMap,
          class NormalMap>
  class Octree {
  public: // types :
    typedef Octree_node<Kernel, PointRange> Node;
    typedef typename Kernel::FT FT;
    typedef typename Kernel::Point_3 Point;
    typedef IntPoint_3 IntPoint;
    typedef typename Kernel::Vector_3 Vector;
    typedef typename Kernel::Iso_cuboid_3 Iso_cuboid;
    typedef typename PointRange::const_iterator InputIterator;
    typedef typename std::list<InputIterator> IterList;

  private: // data members :
    Node m_root;                      /* root node of the octree */
    uint8_t m_max_depth_reached = 0;  /* octree actual highest depth reached */
    PointRange &m_ranges;              /* input point range */
    PointMap m_points_map;          /* property map: `value_type of InputIterator` -> `Point` (Position) */
    Point m_bbox_min;                  /* input bounding box min value */
    FT m_bbox_side;              /* input bounding box side length (cube) */
    std::vector<FT> m_side_per_depth;      /* side length per node's depth */
    std::vector<size_t> m_unit_per_depth; /* number of unit node (smallest) inside one node for each depth for one axis */

  public: // functions :
    Octree(
            PointRange &pwn,
            PointMap &point_map,
            const FT enlarge_ratio = 1.2) :
            m_ranges(pwn),
            m_points_map(point_map) {
      // compute bbox
      Iso_cuboid bbox = CGAL::bounding_box(boost::make_transform_iterator
                                                   (m_ranges.begin(),
                                                    CGAL::Property_map_to_unary_function<PointMap>(
                                                            m_points_map)),
                                           boost::make_transform_iterator
                                                   (m_ranges.end(),
                                                    CGAL::Property_map_to_unary_function<PointMap>(
                                                            m_points_map)));

      Point bbox_centroid = midpoint(bbox.min(), bbox.max());

      // scale bbox
      bbox = bbox.transform(Aff_transformation_3<Kernel>(SCALING, enlarge_ratio));

      // use isotrope bbox
      FT x_len = bbox.xmax() - bbox.xmin();
      FT y_len = bbox.ymax() - bbox.ymin();
      FT z_len = bbox.zmax() - bbox.zmin();
      FT max_len = (x_len < y_len) ? y_len : x_len;
      max_len = (max_len < z_len) ? z_len : max_len;
      bbox = Iso_cuboid(bbox.min(), bbox.min() + max_len * Vector(1.0, 1.0, 1.0));

      // translate bbox back to initial centroid
      Point bbox_transformed_centroid = midpoint(bbox.min(), bbox.max());
      Vector diff_centroid = bbox_centroid - bbox_transformed_centroid;
      bbox = bbox.transform(Aff_transformation_3<Kernel>(TRANSLATION, diff_centroid));

      // save octree attributes
      m_bbox_min = bbox.min();
      m_bbox_side = bbox.max()[0] - m_bbox_min[0];
      for (InputIterator it = pwn.begin(); it != pwn.end(); it++)
        m_root.add_point(it);
    }

    ~Octree() {
      m_root.unsplit();
    }

    // template < typename CellCriteria, typename NormalCriteria > // or other useful criterion
    void refine(size_t max_depth, size_t max_pts_num) {
      if (max_depth < 0 || max_pts_num < 1) {
        CGAL_TRACE_STREAM << "wrong octree refinement criteria\n";
        return;
      }
      for (int i = 0; i <= (int) max_depth; i++)
        m_side_per_depth.push_back(m_bbox_side / (FT) (1 << i));

      refine_recurse(&m_root, max_depth, max_pts_num);

      for (int i = 0; i <= (int) m_max_depth_reached; i++)
        m_unit_per_depth.push_back(1 << (m_max_depth_reached - i));
    }

    void grade() {
      std::queue<Node *> leaf_nodes;
      fill_leaf_queue(&m_root, leaf_nodes);

      while (!leaf_nodes.empty()) {
        Node *node = leaf_nodes.front();
        leaf_nodes.pop();
        if (!node->is_leaf()) continue;

        std::list<Node *> neighbors_to_split = node->find_unbalanced_neighbors_to_split();
        if (!neighbors_to_split.empty()) leaf_nodes.push(node);
        for (Node *neighbor : neighbors_to_split) {
          neighbor->split();
          reassign_points(neighbor);
          for (int child_id = 0; child_id < 8; child_id++) {
            Node *neighbor_child = neighbor->child(child_id);
            leaf_nodes.push(neighbor_child);
          }
        }
      }
    }

    Node *root() { return &m_root; }

    const Node *root() const { return &m_root; }

  private: // functions :

    Point compute_barycenter_position(Node *node) const {
      FT size = m_side_per_depth[node->depth()];
      FT bary[3];
      for (int i = 0; i < 3; i++)
        bary[i] = node->location()[i] * size + (size / 2.0) + m_bbox_min[i];
      return Point(bary[0], bary[1], bary[2]);
    }

    void refine_recurse(Node *node, size_t dist_to_max_depth, size_t max_pts_num) {
      if (dist_to_max_depth == 0 || node->num_points() <= max_pts_num) {
        if (m_max_depth_reached < node->depth()) m_max_depth_reached = node->depth();
        return;
      }

      node->split();
      reassign_points(node);
      for (int child_id = 0; child_id < 8; child_id++) {
        refine_recurse(node->child(child_id), dist_to_max_depth - 1, max_pts_num);
      }
    }

    void reassign_points(Node *node) {
      Point barycenter = compute_barycenter_position(node);

      for (const InputIterator &pwn_it : node->points()) {
        const Point &point = get(m_points_map, *pwn_it);

        int is_right = barycenter[0] < point[0];
        int is_up = barycenter[1] < point[1];
        int is_front = barycenter[2] < point[2];

        bool equal_right = std::abs(barycenter[0] - point[0]) < 1e-6;
        bool equal_up = std::abs(barycenter[1] - point[1]) < 1e-6;
        bool equal_front = std::abs(barycenter[2] - point[2]) < 1e-6;

        int child_id = (is_front << 2) | (is_up << 1) | is_right;
        node->child(child_id)->add_point(pwn_it);

        if (equal_right) {
          int sym_child_id = (is_front << 2) | (is_up << 1) | (!is_right);
          node->child(sym_child_id)->add_point(pwn_it);
        }

        if (equal_up) {
          int sym_child_id = (is_front << 2) | (!is_up << 1) | is_right;
          node->child(sym_child_id)->add_point(pwn_it);
        }

        if (equal_front) {
          int sym_child_id = (!is_front << 2) | (is_up << 1) | (!is_right);
          node->child(sym_child_id)->add_point(pwn_it);
        }

      }
    }

  }; // end class Octree

} // namespace CGAL

#endif // CGAL_OCTREE_3_H