/*!
  \ingroup PkgSurfaceMeshTopologyConcepts
  \cgalConcept

  The concept `PolygonalSchema` defines a 2D polygonal schema, i.e. a combinatorial surface with labeled edges. A PolygonalSchema is created incrementally by adding facets one at a time. A label is any word that does not contain any space.

  PolygonalSchema::Dart_info should be a class having a public data member char* m_label.
  PolygonalSchema::dimension should be equal to 2.
  
  \cgalRefines GenericMap

  \cgalHasModel \link CGAL::Surface_mesh_topology::Polygonal_schema_with_combinatorial_map `CGAL::Surface_mesh_topology::Polygonal_schema_with_combinatorial_map<Items,Alloc>`\endlink
  \cgalHasModel \link CGAL::Surface_mesh_topology::Polygonal_schema_with_generalized_map `CGAL::Surface_mesh_topology::Polygonal_schema_with_generalized_map<Items,Alloc>`\endlink
  */

class PolygonalSchema
{
public:
  /*! creates an empty `PolygonalSchema` object.
   */
  PolygonalSchema();
    
  /// starts a new surface
  void init_surface();
  
  /// finishes the current surface. Returns one dart of the created surface.
  /// @pre A surface is under creation.
  Dart_handle finish_surface();
  
  /// starts a new facet.
  void init_facet();
  
  /// finishes the current facet. Returns the first dart of this facet.
  /// @pre A facet is under creation.
  Dart_handle finish_facet();
  
  /// adds one edge to the current facet, given by its label `l` (any string containing no space, using minus sign for orientation).
  /// Since the surface is oriented, each label can be used only twice with opposite signs. If this method is called with a label already used, with same sign, an error message is given and this label is ignored. 
  /// @pre A facet is under creation.
  void add_edge_to_facet(const std::string& l);
  
  /// adds the given edges to the current facet.
  /// `s` is a sequence of labels, separated by spaces. All the corresponding edges are added into the current facet.
  /// @pre A facet is under creation.
  void add_edges_to_facet(const std::string& s);
  
  /// adds directly one facet giving the sequence of labels `s` of all its edges (labels are separated by spaces).
  /// @pre A surface is under creation.
  void add_facet(const std::string& s);

  /// returns the label of dart `dh`.
  std::string get_label(Dart_handle dh) const;

  /// returns dart with label `s`, NULL if this label is not used.
  Dart_handle get_dart_labeled(const std::string & s) const;
};
