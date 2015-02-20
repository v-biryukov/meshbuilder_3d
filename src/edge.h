/*****************************************************************************
* name: edge.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Class for representing a figure's edge
*
* license: GPLv3
*
*****************************************************************************/

#pragma once
#include <vector>
#include "point.h"
#include "settings.h"
namespace swift
{
    struct edge
    {
        int start_point;
        int finish_point;
        std::vector< int > points;

        inline edge( int s, int f )
        {
            start_point = s;
            finish_point = f;
        }

        inline bool operator==(const edge& A) const
        { return ((start_point == A.start_point && finish_point == A.finish_point) || (start_point == A.finish_point && finish_point == A.start_point)); }

        void make_triangulation( std::vector< point > & v, REAL av_step )
        {

            points.push_back(start_point);

            // set intermediate points
            REAL norm = (v.at(start_point)-v.at(finish_point)).norm();
            int n = int(ceil(norm / av_step));
            point dif = (v.at(finish_point)-v.at(start_point)) / n;
            for (int i = 1; i < n; i++)
            {
                point p = v.at(start_point) + dif * i;
                v.push_back(p);
                points.push_back(v.size()-1);
            }

            points.push_back(finish_point);
        }
    };
};
