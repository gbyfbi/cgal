namespace CGAL {

/*!
\ingroup PkgConvexHull3Functions

\brief computes robustly the intersection of the halfspaces defined by the planes contained in the range [`begin`, `end`) without constructing the dual points. The result is stored in the polyhedron `P`.
`origin` is a point strictly inside the polyhedron.
This version does not compute the dual points, but instead it uses a traits class for handling predicates for dual points without computing them.

\attention Halfspaces are considered as lower halfspaces that is to say if the plane's equation is \f$ a\, x +b\, y +c\, z + d = 0 \f$ then the corresponding halfspace is defined by \f$ a\, x +b\, y +c\, z + d \le 0 \f$ .
\attention The value type of PlaneIterator (Plane) and the type of the origin (Point_3) must come from the same CGAL kernel.

\pre `origin` is inside the intersection of halfspaces defined by the range [`begin`, `end`).
\pre The computed intersection must be a bounded convex polyhedron.

\tparam PlaneIterator must be an input iterator where the value type must be Polyhedron::Traits::Plane
\tparam Polyhedron must be a model of `ConvexHullPolyhedron_3`.

\sa `halfspace_intersection_with_constructions_3()` 
 */

template <class PlaneIterator, class Polyhedron>
void halfspace_intersection_3 (PlaneIterator begin, PlaneIterator end,
                               Polyhedron &P,
                               typename Polyhedron::Vertex::Point_3 const& origin);

} /* namespace CGAL */
