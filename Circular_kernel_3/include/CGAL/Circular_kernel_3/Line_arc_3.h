// Copyright (c) 2005  INRIA Sophia-Antipolis (France) 
// All rights reserved.
//
// Authors : Monique Teillaud <Monique.Teillaud@sophia.inria.fr>
//           Sylvain Pion     <Sylvain.Pion@sophia.inria.fr>
//           Julien Hazebrouck
//           Damien Leroy
// 
// Partially supported by the IST Programme of the EU as a Shared-cost
// RTD (FET Open) Project under Contract No  IST-2000-26473 
// (ECG - Effective Computational Geometry for Curves and Surfaces) 
// and a STREP (FET Open) Project under Contract No  IST-006413 
// (ACS -- Algorithms for Complex Shapes)

#ifndef CGAL_SPHERICAL_KERNEL_LINE_ARC_3_H
#define CGAL_SPHERICAL_KERNEL_LINE_ARC_3_H

#include<CGAL/utility.h>

namespace CGAL {
  namespace CGALi{
    template <class SK> class Line_arc_3 {

      typedef typename SK::Plane_3              Plane_3;
      typedef typename SK::Sphere_3             Sphere_3;
      typedef typename SK::Point_3              Point_3;
      typedef typename SK::Segment_3            Segment_3;
      typedef typename SK::Circular_arc_point_3 Circular_arc_point_3;
      typedef typename SK::Line_3               Line_3;
      typedef typename SK::FT                   FT;

    private:
      typedef Triple<Line_3, Circular_arc_point_3, 
                             Circular_arc_point_3>  Rep;
      typedef typename SK::template Handle<Rep>::type  Base;

      Base base;
      mutable unsigned char begin_less_xyz_than_end_flag;

      bool begin_less_xyz_than_end() const {
        if(begin_less_xyz_than_end_flag == 0) {
          if(SK().compare_xyz_3_object()(source(), target()) < 0)
            begin_less_xyz_than_end_flag = 2;
          else begin_less_xyz_than_end_flag = 1;
        } return begin_less_xyz_than_end_flag == 2;
      }

    public:
      Line_arc_3()
      : begin_less_xyz_than_end_flag(0) 
      {}

      Line_arc_3(const Line_3 &l, 
                 const Circular_arc_point_3 &s,
                 const Circular_arc_point_3 &t) 
      : begin_less_xyz_than_end_flag(0)
      {
        // l must pass through s and t, and s != t
        CGAL_kernel_precondition(SK().has_on_3_object()(l,s));
        CGAL_kernel_precondition(SK().has_on_3_object()(l,t));
        CGAL_kernel_precondition(s != t);
        base = Rep(l,s,t);
      }

      Line_arc_3(const Segment_3 &s) 
      : begin_less_xyz_than_end_flag(0)
      {
        base = Rep(s.supporting_line(),
                   s.source(),
                   s.target());
      }

      Line_arc_3(const Line_3 &l, 
                 const Point_3 &s,
                 const Point_3 &t) 
      : begin_less_xyz_than_end_flag(0)
      {
        // l must pass through s and t, and s != t
        CGAL_kernel_precondition(SK().has_on_3_object()(l,s));
        CGAL_kernel_precondition(SK().has_on_3_object()(l,t));
        CGAL_kernel_precondition(Circular_arc_point_3(s) != 
                                 Circular_arc_point_3(t));
        base = Rep(l,s,t);
      }

      Line_arc_3(const Line_3 &l, 
                 const Point_3 &s,
                 const Circular_arc_point_3 &t) 
      : begin_less_xyz_than_end_flag(0)
      {
        // l must pass through s and t, and s != t
        CGAL_kernel_precondition(SK().has_on_3_object()(l,s));
        CGAL_kernel_precondition(SK().has_on_3_object()(l,t));
        CGAL_kernel_precondition(Circular_arc_point_3(s) != t);
        base = Rep(l,s,t);
      }

      Line_arc_3(const Line_3 &l, 
                 const Circular_arc_point_3 &s,
                 const Point_3 &t) 
      : begin_less_xyz_than_end_flag(0)
      {
        // l must pass through s and t, and s != t
        CGAL_kernel_precondition(SK().has_on_3_object()(l,s));
        CGAL_kernel_precondition(SK().has_on_3_object()(l,t));
        CGAL_kernel_precondition(s != Circular_arc_point_3(t));
        base = Rep(l,s,t);
      }

      Line_arc_3(const Line_3 &l, 
                 const Sphere_3 &s,
                 bool less_xyz_first = true) 
      {
         std::vector<Object> sols;
         SK().intersect_3_object()(l, s, std::back_inserter(sols));
         // l must intersect s in 2 points 
         std::pair<typename SK::Circular_arc_point_3, unsigned> pair1, pair2;
         CGAL_kernel_precondition(sols.size() == 2);
         assign(pair1,sols[0]);
         assign(pair2,sols[1]);
         if(less_xyz_first) {
           *this = Line_arc_3(l, pair1.first, pair2.first);
         } else {
           *this = Line_arc_3(l, pair2.first, pair1.first);
         } 
      }

      Line_arc_3(const Line_3 &l, 
                 const Sphere_3 &s1, bool less_xyz_s1,
                 const Sphere_3 &s2, bool less_xyz_s2) 
      {
         std::vector<Object> sols1, sols2;
         SK().intersect_3_object()(l, s1, std::back_inserter(sols1));
         SK().intersect_3_object()(l, s2, std::back_inserter(sols2));
         std::pair<typename SK::Circular_arc_point_3, unsigned> pair1, pair2;
         // l must intersect s1 and s2
         CGAL_kernel_precondition(sols1.size() > 0);
         CGAL_kernel_precondition(sols2.size() > 0);
         assign(pair1,sols1[(sols1.size()==1)?(0):(less_xyz_s1?0:1)]);
         assign(pair2,sols2[(sols2.size()==1)?(0):(less_xyz_s2?0:1)]);
         // the source and target must be different
         CGAL_kernel_precondition(pair1.first != pair2.first);
         *this = Line_arc_3(l, pair1.first, pair2.first);
      }

      Line_arc_3(const Line_3 &l, 
                 const Plane_3 &p1,
                 const Plane_3 &p2) 
      {
         // l must not be on p1 or p2
         CGAL_kernel_precondition(!SK().has_on_3_object()(p1,l));
         CGAL_kernel_precondition(!SK().has_on_3_object()(p2,l));
         typename SK::Point_3 point1, point2;
         // l must intersect p1 and p2
         assert(assign(point1,SK().intersect_3_object()(l, p1)));
         assert(assign(point2,SK().intersect_3_object()(l, p2)));
         assign(point1,SK().intersect_3_object()(l, p1));
         assign(point2,SK().intersect_3_object()(l, p2));
         // the source and target must be different
         CGAL_kernel_precondition(point1 != point2);
         *this = Line_arc_3(l, point1, point2);
      }

      const Line_3& supporting_line() const 
      {
        return get(base).first;
      }

      const Circular_arc_point_3& source() const 
      {
        return get(base).second;
      }

      const Circular_arc_point_3& target() const 
      {
        return get(base).third;
      }

      const Circular_arc_point_3& lower_xyz_extremity() const
      {
        return begin_less_xyz_than_end() ? source() : target();
      }

      const Circular_arc_point_3& higher_xyz_extremity() const
      {
        return begin_less_xyz_than_end() ? target() : source();
      }

      const CGAL::Bbox_3 bbox() const {
        return source().bbox() + target().bbox();
      }

      bool operator==(const Line_arc_3 &) const;
      bool operator!=(const Line_arc_3 &) const;

    };

    template < class SK >
    CGAL_KERNEL_INLINE
    bool
    Line_arc_3<SK>::operator==(const Line_arc_3<SK> &t) const
    {
      if (CGAL::identical(base, t.base))
        return true;
      return CGAL::SphericalFunctors::non_oriented_equal<SK>(*this, t);
    }

    template < class SK >
    CGAL_KERNEL_INLINE
    bool
    Line_arc_3<SK>::operator!=(const Line_arc_3<SK> &t) const
    {
      return !(*this == t);
    }

  }
}

#endif

