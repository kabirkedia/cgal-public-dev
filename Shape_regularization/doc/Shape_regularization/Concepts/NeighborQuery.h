/*!
\ingroup PkgShapeRegularizationRefConcepts
\cgalConcept

A concept that describes the set of methods used by the class 
`CGAL::Shape_regularization::QP_regularization` 
to access the neighbors of an item.

\cgalHasModel 
- `CGAL::Shape_regularization::Segments::Delaunay_neighbor_query_2`
*/
class NeighborQuery {

public:

  /*!  
    fills `neighbors` with the indices of all items, which are connected to the 
    item with the index `query_index`.

    `CGAL::Shape_regularization::QP_regularization` calls this function once per each item.
  */
  void operator()(
    const std::size_t query_index, 
    std::vector<std::size_t>& neighbors) {
  
  }
};