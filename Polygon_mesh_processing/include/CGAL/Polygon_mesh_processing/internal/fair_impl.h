#ifndef CGAL_POLYGON_MESH_PROCESSING_FAIR_POLYHEDRON_3_H
#define CGAL_POLYGON_MESH_PROCESSING_FAIR_POLYHEDRON_3_H

#include <map>
#include <set>
#include <CGAL/assertions.h>
#include <CGAL/Polygon_mesh_processing/internal/Hole_filling/Weights.h>
#include <CGAL/Timer.h>
#include <CGAL/trace.h>
#include <iterator>

namespace CGAL {

namespace Polygon_mesh_processing {

namespace internal {

// [On Linear Variational Surface Deformation Methods-2008]
template<class PolygonMesh,
         class SparseLinearSolver,
         class WeightCalculator,
         class VertexPointMap>
class Fair_Polyhedron_3 {
  // typedefs
  typedef typename VertexPointMap::value_type Point_3;
  typedef typename boost::graph_traits<PolygonMesh>::vertex_descriptor vertex_descriptor;
  typedef  Halfedge_around_target_circulator<PolygonMesh>  Halfedge_around_vertex_circulator;

  typedef SparseLinearSolver Sparse_linear_solver;
  typedef typename Sparse_linear_solver::Matrix Solver_matrix;
  typedef typename Sparse_linear_solver::Vector Solver_vector;

// members
  PolygonMesh& pmesh;
  Sparse_linear_solver m_solver;
  WeightCalculator weight_calculator;
  VertexPointMap ppmap;

public:
  Fair_Polyhedron_3(PolygonMesh& pmesh
      , VertexPointMap vpmap
      , WeightCalculator weight_calculator)
    : pmesh(pmesh)
    , weight_calculator(weight_calculator)
    , ppmap(vpmap)
  { }
  
private:
  double sum_weight(vertex_descriptor v) {
  double weight = 0;
  Halfedge_around_vertex_circulator circ(halfedge(v,pmesh),pmesh), done(circ);
  do {
    weight += weight_calculator.w_ij(*circ);
    } while(++circ != done);
    return weight;
  }

  // recursively computes a row (use depth parameter to compute L, L^2, L^3)
  // Equation 6 in [On Linear Variational Surface Deformation Methods]
  void compute_row(
    vertex_descriptor v,
    int row_id,                            // which row to insert in [ frees stay left-hand side ]
    Solver_matrix& matrix, 
    double& x, double& y, double& z,               // constants transfered to right-hand side
    double multiplier,
    const std::map<vertex_descriptor, std::size_t>& vertex_id_map,
    unsigned int depth)
  {
    if(depth == 0) {
      typename std::map<vertex_descriptor, std::size_t>::const_iterator vertex_id_it = vertex_id_map.find(v);
      if(vertex_id_it != vertex_id_map.end()) {
        int col_id = static_cast<int>(vertex_id_it->second);
        matrix.add_coef(row_id, col_id, multiplier);
      }
      else { 
        Point_3& p = ppmap[v];
        x += multiplier * - p.x(); 
        y += multiplier * - p.y(); 
        z += multiplier * - p.z(); 
      }
    }
    else {
      double w_i = weight_calculator.w_i(v);

      Halfedge_around_vertex_circulator circ(halfedge(v,pmesh),pmesh), done(circ);
      do {
        double w_i_w_ij = w_i * weight_calculator.w_ij(*circ) ;

        vertex_descriptor nv = target(opposite(*circ,pmesh),pmesh);
        compute_row(nv, row_id, matrix, x, y, z, -w_i_w_ij*multiplier, vertex_id_map, depth-1);
      } while(++circ != done);

      double w_i_w_ij_sum = w_i * sum_weight(v);
      compute_row(v, row_id, matrix, x, y, z, w_i_w_ij_sum*multiplier, vertex_id_map, depth-1);
    }
  }

  template<class VertexRange>
  bool fair_all_mesh(const VertexRange& vr) const
  {
    return std::distance(vr.begin(), vr.end())
        == std::distance(vertices(pmesh).first, vertices(pmesh).second);
  }

  void remove_extremal_vertices(std::set<vertex_descriptor>& vertices) const
  {
    vertex_descriptor v_xmin, v_xmax, v_ymin, v_ymax, v_zmin, v_zmax;
    v_xmin = v_xmax = v_ymin = v_ymax = v_zmin = v_zmax = *(vertices.begin());
    BOOST_FOREACH(vertex_descriptor v, vertices)
    {
      Point_3 pv = ppmap[v];
      if (pv.x() < ppmap[v_xmin].x()) v_xmin = v;
      if (pv.y() < ppmap[v_ymin].y()) v_ymin = v;
      if (pv.z() < ppmap[v_zmin].z()) v_zmin = v;
      if (pv.x() > ppmap[v_xmax].x()) v_xmax = v;
      if (pv.y() > ppmap[v_ymax].y()) v_ymax = v;
      if (pv.z() > ppmap[v_zmax].z()) v_zmax = v;
    }

    //here we use a set to avoid removing twice the same vertex, which would crash
    std::set<vertex_descriptor> to_be_removed;
    to_be_removed.insert(v_xmin);
    to_be_removed.insert(v_ymin);
    to_be_removed.insert(v_zmin);
    to_be_removed.insert(v_xmax);
    to_be_removed.insert(v_ymax);
    to_be_removed.insert(v_zmax);

    //std::cout << ppmap[v_xmin] << std::endl;
    //std::cout << ppmap[v_ymin] << std::endl;
    //std::cout << ppmap[v_zmin] << std::endl;
    //std::cout << ppmap[v_xmax] << std::endl;
    //std::cout << ppmap[v_ymax] << std::endl;
    //std::cout << ppmap[v_zmax] << std::endl;

    BOOST_FOREACH(vertex_descriptor v, to_be_removed)
      vertices.erase(v);
  }

  void remove_vertices(std::set<vertex_descriptor>& vertices
    , const double& percent = 10/*percentage to be removed*/) const
  {
    CGAL_assertion(percent >= 0.1 && percent < 100.);
    int freq = static_cast<int>(std::floor(0.01 * percent * vertices.size()));

    int i = 1;
    typename std::set<vertex_descriptor>::iterator vit;
    for (vit = vertices.begin(); vit != vertices.end(); )
    {
      vertex_descriptor vd = *vit;
      ++vit;
      if (i % freq == 0)
        vertices.erase(vd);
      ++i;
    }
  }

public:
  template<class VertexRange>
  bool fair(const VertexRange& vertices
    , SparseLinearSolver m_solver
    , unsigned int fc)
  {
    int depth = static_cast<int>(fc) + 1;
    if(depth < 0 || depth > 3) {
      CGAL_warning(!"Continuity should be between 0 and 2 inclusively!");
      return false; 
    }

    std::set<vertex_descriptor> interior_vertices(boost::begin(vertices),
                                                  boost::end(vertices));
    if(interior_vertices.empty()) { return true; }

    CGAL::Timer timer; timer.start();

    if (fair_all_mesh(vertices))
      remove_vertices(interior_vertices, 10/*percentage to be removed*/);
      //remove_extremal_vertices(interior_vertices);

    const std::size_t nb_vertices = interior_vertices.size();
    Solver_vector X(nb_vertices), Bx(nb_vertices);
    Solver_vector Y(nb_vertices), By(nb_vertices);
    Solver_vector Z(nb_vertices), Bz(nb_vertices);

    std::map<vertex_descriptor, std::size_t> vertex_id_map;
    std::size_t id = 0;
    BOOST_FOREACH(vertex_descriptor vd, interior_vertices)
    {
      if( !vertex_id_map.insert(std::make_pair(vd, id)).second ) {
        CGAL_warning(!"Duplicate vertex is found!");
        return false;
      }
      ++id;
    }

    Solver_matrix A(nb_vertices);

    BOOST_FOREACH(vertex_descriptor vd, interior_vertices)
    {
      int v_id = static_cast<int>(vertex_id_map[vd]);
      compute_row(vd, v_id, A, Bx[v_id], By[v_id], Bz[v_id], 1, vertex_id_map, depth);
    }
    CGAL_TRACE_STREAM << "**Timer** System construction: " << timer.time() << std::endl; timer.reset();

    // factorize
    double D;
//    Sparse_linear_solver m_solver;
    bool prefactor_ok = m_solver.factor(A, D);
    if(!prefactor_ok) {
      CGAL_warning(!"pre_factor failed!");
      return false;
    }
    CGAL_TRACE_STREAM << "**Timer** System factorization: " << timer.time() << std::endl; timer.reset();

    // solve
    bool is_all_solved = m_solver.linear_solver(Bx, X) && m_solver.linear_solver(By, Y) && m_solver.linear_solver(Bz, Z);
    if(!is_all_solved) {
      CGAL_warning(!"linear_solver failed!"); 
      return false; 
    }
    CGAL_TRACE_STREAM << "**Timer** System solver: " << timer.time() << std::endl; timer.reset();

    
    /* This relative error is to large for cases that the results are not good */ 
    /*
    double rel_err_x = (A.eigen_object()*X - Bx).norm() / Bx.norm();
    double rel_err_y = (A.eigen_object()*Y - By).norm() / By.norm();
    double rel_err_z = (A.eigen_object()*Z - Bz).norm() / Bz.norm();
    CGAL_TRACE_STREAM << "rel error: " << rel_err_x 
                                << " " << rel_err_y
                                << " " << rel_err_z << std::endl;
                                */

    // update 
    id = 0;
    BOOST_FOREACH(vertex_descriptor vd, interior_vertices)
    {
      put(ppmap, vd, Point_3(X[id], Y[id], Z[id]));
      ++id;
    }
    return true;
  }
};

}//namespace internal

}//namespace Polygon_mesh_processing

}//namespace CGAL
#endif //CGAL_POLYGON_MESH_PROCESSING_FAIR_POLYHEDRON_3_H
