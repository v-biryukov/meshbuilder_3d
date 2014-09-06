/*****************************************************************************
* name: point.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Simple point class
*
* license: GPLv3
*
*****************************************************************************/

#pragma once
#include <cmath>
#include <iostream>
#include "settings.h"
namespace swift
{
    struct point
    {
        REAL x, y, z;

        inline point( void ) {}
        inline point( const point& p )
        { x = p.x; y = p.y; z = p.z; }
        inline point( const REAL xt, const REAL yt, const REAL zt )
        { x = xt; y = yt; z = zt; }

        inline point operator + ( const point& A ) const
        { return point( x + A.x, y + A.y, z + A.z ); }

        inline point operator - ( const point& A ) const
        { return point( x - A.x, y - A.y, z - A.z ); }

        inline point operator + () const
        { return point( x, y, z ); }

        inline point operator - () const
        { return point( -x, -y, -z ); }

        inline point operator * ( const REAL A ) const
        { return point( x * A, y * A, z * A ); }

        inline point operator / ( const REAL A ) const
        { return point( x / A, y / A, z / A ); }

        inline bool operator==(const point& A) const
        { return ((*this - A).norm() < 1e-10); }

        inline bool operator != ( const point& A ) const
        { return ((*this - A).norm() >= 1e-10); }

        inline REAL norm_sq( void ) const
        { return x*x + y*y + z*z; }

        inline REAL norm( void ) const
        { return sqrt(x*x + y*y + z*z); }

        inline REAL dot( const point& A )
        { return x*A.x + y*A.y + z*A.z; }

        inline point vec (const point& A )
        { return point( y * A.z - A.y * z, -x * A.z + z * A.x, x * A.y - y * A.x ); }

        REAL projx (point n, point new_x)
        {
            return (*this - n * n.dot(*this)).dot(new_x);
        }
        REAL projy (point n, point new_x)
        {
            return (*this - n * n.dot(*this)).vec(new_x).dot(n);
        }

        point rotate(REAL a, REAL b, REAL g) const
        {
            return point ((cos(a)*cos(g)-sin(a)*cos(b)*sin(g))*x + (-cos(a)*sin(g)-sin(a)*cos(b)*cos(g))*y + sin(a)*sin(b)*z,
                          (sin(a)*cos(g)+cos(a)*cos(b)*sin(g))*x + (-sin(a)*sin(g)+cos(a)*cos(b)*cos(g))*y - cos(a)*sin(b)*z,
                          (sin(b)*sin(g)*x + sin(b)*cos(g)*y + cos(b)*z));
        }

        friend std::ostream& operator<<(std::ostream& os, const point& p);
    };

    point get_by_proj(point n, REAL x, REAL y, point old_x)
    {
        point old_y = old_x.vec(n);
        return old_x * x + old_y * y;
    }

    REAL det(point a, point b, point c)
    {
        return a.x*(b.y*c.z - c.y*b.z) + b.x*(c.y*a.z - a.y*c.z) + c.x*(a.y*b.z - b.y*a.z);
    }

    std::ostream& operator<<(std::ostream& os, const point& p)
    {
        os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
        return os;
    }
}
