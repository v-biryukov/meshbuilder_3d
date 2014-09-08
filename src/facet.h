/*****************************************************************************
* name: facet.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Class for representing a figure's facet
*
* license: GPLv3
*
*****************************************************************************/

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include "point.h"
#include "edge.h"
#include "settings.h"
#include <boost/foreach.hpp>
extern "C"
{
#include "triangle.h"
}
namespace swift
{
    struct trifacet
    {
        int points[3];
        trifacet( int p1, int p2, int p3 )
        {
            points[0] = p1;
            points[1] = p2;
            points[2] = p3;
        }
    };

    struct facet
    {
        std::vector< int > edges;
        std::vector< int > points;
        std::vector< int > trifacets;

        struct triangulateio in, out;

        facet(std::vector<int> e)
        {
            points = e;
        }

        facet(int e1, int e2, int e3)
        {
            points.push_back(e1);
            points.push_back(e2);
            points.push_back(e3);
        }

        facet(int e1, int e2, int e3, int e4)
        {
            points.push_back(e1);
            points.push_back(e2);
            points.push_back(e3);
            points.push_back(e4);
        }

        void add_corner_points(std::vector<edge> & ve)
        {
            for(std::vector<int>::size_type i = 0; i != edges.size(); i++)
            {
                if(std::find(points.begin(), points.end(), ve.at(edges[i]).start_point) == points.end())
                    points.push_back(ve.at(edges[i]).start_point);
                if(std::find(points.begin(), points.end(), ve.at(edges[i]).finish_point) == points.end())
                    points.push_back(ve.at(edges[i]).finish_point);
            }
        }

        void add_edges_by_points(std::vector<edge> & ve)
        {
            for (unsigned int i = 0; i < points.size(); i++)
            {
                edge e = edge(points.at(i), points.at((i+1) % points.size()));
                if (std::find(ve.begin(), ve.end(), e) == ve.end())
                {
                    ve.push_back(e);
                    edges.push_back(ve.size()-1);
                }
                else
                {
                    edges.push_back(std::distance(ve.begin(), std::find(ve.begin(), ve.end(), e)));
                }
            }
        }

        void add_segments_from_edges (std::vector<int> &vs, std::vector<edge> & ve)
        {
            for (std::vector<int>::iterator e = edges.begin(); e != edges.end(); e++)
                for (std::vector<int>::iterator it = ve.at(*e).points.begin();it != ve.at(*e).points.end() - 1; it++)
                    {
                        int p1 = std::distance(points.begin(), std::find(points.begin(), points.end(), *it));
                        int p2 = std::distance(points.begin(), std::find(points.begin(), points.end(), *(it+1)));
                        vs.push_back(p1);
                        vs.push_back(p2);
                    }
        }

        void make_triangulation( std::vector< point > & vp, std::vector< trifacet > & vt, std::vector<edge> & ve, REAL av_step = 0, REAL (*f)(REAL,REAL,REAL) = 0 )
        {
            add_edges_by_points(ve);
            //add_corner_points(ve);
            // Creating points on all edges
            BOOST_FOREACH(int e, edges)
            {
                if (ve.at(e).points.empty())
                {
                    ve.at(e).make_triangulation(vp, av_step, f);
                }
                for(std::vector<int>::iterator jt = ve.at(e).points.begin()+1; jt != ve.at(e).points.end() - 1; jt++)
                {
                    points.push_back(*jt);
                }
            }
            // Calculating three main points in order to calculate a normal
            assert (points.size() >= 3);
            std::vector<point> main_points;
            main_points.push_back(vp.at(points.at(0)));
            main_points.push_back(vp.at(points.at(1)));
            main_points.push_back(vp.at(points.at(2)));
            point normal = (main_points[0] - main_points[1]).vec(main_points[0] - main_points[2]);
            normal = normal / normal.norm();
            // Setting triangulateio in and out:
            in.numberofpoints = points.size();
            in.numberofpointattributes = 0;
            in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
            in.pointmarkerlist = (int *) NULL;
            in.pointattributelist = (REAL *) NULL;
            point p0 = main_points[1] - main_points[0];
            p0 = p0 / p0.norm();
            for (int i = 0; i < in.numberofpoints; i++)
            {
                in.pointlist[2*i + 0] = (vp.at(points[i])).projx(normal, p0);
                in.pointlist[2*i + 1] = (vp.at(points[i])).projy(normal, p0);
                std::cout << i << " : " << in.pointlist[2*i + 0] << "  " << in.pointlist[2*i + 1] << "\n";
            }

            in.numberofsegments = in.numberofpoints;
            in.segmentlist = (int *) malloc(in.numberofsegments * 2 * sizeof(int));
            std::vector<int> temp_vec;
            add_segments_from_edges(temp_vec, ve);
            std::copy(temp_vec.begin(), temp_vec.end(),in.segmentlist);
            for (int i = 0; i < in.numberofsegments; i++)
            {
                std::cout << i << " : " << in.segmentlist[2*i + 0] << "  " << in.segmentlist[2*i + 1] << "\n";
            }
            in.numberofholes = 0;
            in.numberofregions = 0;
            in.holelist = (REAL *) NULL;
            in.regionlist = (REAL *) NULL;

            out.pointlist = (REAL *) NULL;
            out.pointattributelist = (REAL *) NULL;
            out.pointmarkerlist = (int *) NULL;
            out.trianglelist = (int *) NULL;
            out.triangleattributelist = (REAL *) NULL;
            out.neighborlist = (int *) NULL;
            out.segmentlist = (int *) NULL;
            out.segmentmarkerlist = (int *) NULL;
            out.edgelist = (int *) NULL;
            out.edgemarkerlist = (int *) NULL;


            if (f == 0)
            {
                std::stringstream ss;
                ss << "pzqYa" << av_step * av_step / 2;
                std::string str;
                ss >> str;
                char * s = new char[str.size() + 1];
                std::copy(str.begin(), str.end(), s);
                s[str.size()] = '\0';
                triangulate(s, &in, &out, (struct triangulateio *) NULL);
            }
            else
            {
                struct triangulateio mid;
                mid.pointlist = (REAL *) NULL;
                mid.pointattributelist = (REAL *) NULL;
                mid.pointmarkerlist = (int *) NULL;
                mid.trianglelist = (int *) NULL;
                mid.triangleattributelist = (REAL *) NULL;
                mid.neighborlist = (int *) NULL;
                mid.segmentlist = (int *) NULL;
                mid.segmentmarkerlist = (int *) NULL;
                mid.edgelist = (int *) NULL;
                mid.edgemarkerlist = (int *) NULL;
                std::stringstream ss;
                ss << "pzqYa" << av_step * av_step / 2;
                std::string str;
                ss >> str;
                char * s = new char[str.size() + 1];
                std::copy(str.begin(), str.end(), s);
                s[str.size()] = '\0';
                triangulate(s, &in, &mid, (struct triangulateio *) NULL);
                mid.trianglearealist = (REAL *) malloc(mid.numberoftriangles * sizeof(REAL));
                for (int i = 0; i < mid.numberoftriangles; i++)
                {
                    point p = normal * normal.dot(main_points[0]) + get_by_proj(normal, mid.pointlist[2*mid.trianglelist[i]], mid.pointlist[2*mid.trianglelist[i]+1], p0);
                    mid.trianglearealist[i] = f(p.x, p.y, p.z)*f(p.x, p.y, p.z)*av_step*av_step/2;
                }
                triangulate((char*)"rzqYa", &mid, &out, (struct triangulateio *) NULL);
            }

            for (int i = in.numberofpoints; i < out.numberofpoints; i++)
            {
                vp.push_back(normal * normal.dot(main_points[0]) + get_by_proj(normal, out.pointlist[2*i], out.pointlist[2*i+1], p0));
                points.push_back(vp.size()-1);
            }

            for (int i = 0; i < out.numberoftriangles; i++)
            {
                vt.push_back(trifacet(points[out.trianglelist[3*i]], points[out.trianglelist[3*i + 1]], points[out.trianglelist[3*i + 2]]));
                trifacets.push_back(vt.size()-1);
            }
        }

        void take_triangulation( std::vector< point > & vp, std::vector< trifacet > & vt, std::vector<edge> & ve, facet & f)
        {
            using std::vector;
            using std::find;

            add_edges_by_points(ve);

            vector<point> main_points;
            BOOST_FOREACH (int e, edges)
            {
                point sp = vp.at(ve.at(e).start_point);
                point fp = vp.at(ve.at(e).finish_point);
                if (find(main_points.begin(), main_points.end(), sp) == main_points.end())
                    main_points.push_back(sp);
                if (find(main_points.begin(), main_points.end(), fp) == main_points.end())
                    main_points.push_back(fp);
            }

            vector<point> f_points;
            BOOST_FOREACH(int e, f.edges)
            {
                point sp = vp.at(ve.at(e).start_point);
                point fp = vp.at(ve.at(e).finish_point);
                if (find(f_points.begin(), f_points.end(), sp) == f_points.end())
                    f_points.push_back(sp);
                if (find(f_points.begin(), f_points.end(), fp) == f_points.end())
                    f_points.push_back(fp);
            }
            point normal = (main_points[0] - main_points[1]).vec(main_points[0] - main_points[2]);
            normal = normal / normal.norm();
            point dif = point(0, 0, 0);
            BOOST_FOREACH (point p, main_points)
                dif = dif + p;
            BOOST_FOREACH (point p, f_points)
                dif = dif - p;
            dif = dif / dif.norm();

            for (std::vector<int>::size_type i = 0; i < f.edges.size(); i++)
            {
                BOOST_FOREACH(int pe, ve.at(f.edges.at(i)).points)
                {
                    point p = project(vp.at(pe), dif, normal, (main_points.at(0) + main_points.at(1) + main_points.at(2))/3 );
                    std::vector<point>::iterator it = find(vp.begin(), vp.end(), p);
                    if (it == vp.end())
                    {
                        vp.push_back(p);
                        ve.at(edges.at(i)).points.push_back(vp.size()-1);
                    }
                    else
                    {
                        ve.at(edges.at(i)).points.push_back(std::distance(vp.begin(), it));
                    }
                }
            }
            for (int i = 0; i < f.out.numberofpoints; i++)
            {
                point p = project(vp.at(f.points.at(i)), dif, normal, (main_points.at(0) + main_points.at(1) + main_points.at(2))/3 );
                std::vector<point>::iterator it = find(vp.begin(), vp.end(), p);
                if (it == vp.end())
                {
                    vp.push_back(p);
                    points.push_back(vp.size()-1);
                }
                else
                {
                    if (find(points.begin(), points.end(), std::distance(vp.begin(), it)) == points.end())
                        points.push_back(std::distance(vp.begin(), it));
                }
            }
            for (int i = 0; i < f.out.numberoftriangles; i++)
            {
                vt.push_back(trifacet(points[f.out.trianglelist[3*i]], points[f.out.trianglelist[3*i + 1]], points[f.out.trianglelist[3*i + 2]]));
                trifacets.push_back(vt.size()-1);
            }

        }

        point project(point p0, point dif, point normal, point m0)
        {
            return p0 + dif * (m0 - p0).dot(normal) / dif.dot(normal);
        }
    };
}
