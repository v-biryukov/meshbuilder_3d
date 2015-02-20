#pragma once
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
        std::vector< trifacet > full_trifacets;

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

        void make_triangulation( std::vector< point > & vp, std::vector< trifacet > & vt, std::vector<edge> & ve, REAL av_step = 0)
        {
            add_edges_by_points(ve);
            //add_corner_points(ve);
            // Creating points on all edges
            for ( unsigned int i = 0; i < edges.size(); i++ )
            {
                int e = edges.at(i);
                if (ve.at(e).points.empty())
                {
                    ve.at(e).make_triangulation(vp, av_step);
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
            point p0 = main_points[1] - main_points[0];
            p0 = p0 / p0.norm();

            //if (use_triangle_over_fade)
            {
                struct triangulateio in, out;
                // Setting triangulateio in and out:

                in.pointlist = (REAL*)(NULL);
				in.pointattributelist = (REAL*)(NULL);
				in.pointmarkerlist = (int*)(NULL);
				in.numberofpoints = 0;
				in.numberofpointattributes = 0;

				in.trianglelist = (int*)(NULL);
				in.triangleattributelist = (REAL*)(NULL);
				in.trianglearealist = (REAL*)(NULL);
				in.neighborlist = (int*)(NULL);
				in.numberoftriangles = 0;
				in.numberofcorners = 0;
				in.numberoftriangleattributes = 0;

				in.segmentlist = (int*)(NULL);
				in.segmentmarkerlist = (int*)(NULL);
				in.numberofsegments = 0;

				in.holelist = (REAL*)(NULL);
				in.numberofholes = 0;

				in.regionlist = (REAL*)(NULL);
				in.numberofregions = 0;

				in.edgelist = (int*)(NULL);
				in.edgemarkerlist = (int*)(NULL);
				in.normlist = (REAL*)(NULL);
				in.numberofedges = 0;

                in.numberofpoints = points.size();
                in.numberofpointattributes = 0;
                in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
                in.pointmarkerlist = (int *) NULL;
                in.pointattributelist = (REAL *) NULL;

                for (int i = 0; i < in.numberofpoints; i++)
                {
                    in.pointlist[2*i + 0] = (vp.at(points[i])).projx(normal, p0);
                    in.pointlist[2*i + 1] = (vp.at(points[i])).projy(normal, p0);
                    //std::cout << "" << in.pointlist[2*i + 0] << " " << in.pointlist[2*i + 1] << std::endl;
                }

                in.numberofsegments = in.numberofpoints;
                in.segmentlist = (int *) malloc(in.numberofsegments * 2 * sizeof(int));
				in.segmentmarkerlist = (int *) NULL;
                std::vector<int> temp_vec;
                add_segments_from_edges(temp_vec, ve);
                std::copy(temp_vec.begin(), temp_vec.end(),in.segmentlist);


                out.pointlist = (REAL*)(NULL);
				out.pointattributelist = (REAL*)(NULL);
				out.pointmarkerlist = (int*)(NULL);
				out.numberofpoints = 0;
				out.numberofpointattributes = 0;

				out.trianglelist = (int*)(NULL);
				out.triangleattributelist = (REAL*)(NULL);
				out.trianglearealist = (REAL*)(NULL);
				out.neighborlist = (int*)(NULL);
				out.numberoftriangles = 0;
				out.numberofcorners = 0;
				out.numberoftriangleattributes = 0;

				out.segmentlist = (int*)(NULL);
				out.segmentmarkerlist = (int*)(NULL);
				out.numberofsegments = 0;

				out.holelist = (REAL*)(NULL);
				out.numberofholes = 0;

				out.regionlist = (REAL*)(NULL);
				out.numberofregions = 0;

				out.edgelist = (int*)(NULL);
				out.edgemarkerlist = (int*)(NULL);
				out.normlist = (REAL*)(NULL);
				out.numberofedges = 0;

                std::stringstream ss5;
                ss5 << "pzqQYa" << av_step * av_step / 2;
                std::string str;
                ss5 >> str;
                char * s = new char[str.size() + 1];
                std::copy(str.begin(), str.end(), s);
                s[str.size()] = '\0';
                triangulate(s, &in, &out, (struct triangulateio *) NULL);


                for (int i = in.numberofpoints; i < out.numberofpoints; i++)
                {
                    vp.push_back(normal * normal.dot(main_points[0]) + get_by_proj(normal, out.pointlist[2*i], out.pointlist[2*i+1], p0));
                    points.push_back(vp.size()-1);
                }

                for (int i = 0; i < out.numberoftriangles; i++)
                {
                    vt.push_back(trifacet(points[out.trianglelist[3*i]], points[out.trianglelist[3*i + 1]], points[out.trianglelist[3*i + 2]]));
                    trifacets.push_back(vt.size()-1);
                    full_trifacets.push_back(trifacet(out.trianglelist[3*i], out.trianglelist[3*i + 1], out.trianglelist[3*i + 2]));
                }
				

				if (in.pointlist)             free(in.pointlist);
			    if (in.pointattributelist)    free(in.pointattributelist);
			    if (in.pointmarkerlist)       free(in.pointmarkerlist);
			    if (in.trianglelist)          free(in.trianglelist);
			    if (in.triangleattributelist) free(in.triangleattributelist);
			    if (in.trianglearealist)      free(in.trianglearealist);
			    if (in.neighborlist)          free(in.neighborlist);
			    if (in.segmentlist)           free(in.segmentlist);
			    if (in.segmentmarkerlist)     free(in.segmentmarkerlist);
			    if (in.regionlist)            free(in.regionlist);
			    if (in.edgelist)              free(in.edgelist);
			    if (in.edgemarkerlist)        free(in.edgemarkerlist);
			    if (in.normlist)              free(in.normlist);

				if (out.pointlist)             free(out.pointlist);
			    if (out.pointattributelist)    free(out.pointattributelist);
			    if (out.pointmarkerlist)       free(out.pointmarkerlist);
			    if (out.trianglelist)          free(out.trianglelist);
			    if (out.triangleattributelist) free(out.triangleattributelist);
			    if (out.trianglearealist)      free(out.trianglearealist);
			    if (out.neighborlist)          free(out.neighborlist);
			    if (out.segmentlist)           free(out.segmentlist);
			    if (out.segmentmarkerlist)     free(out.segmentmarkerlist);
			    if (out.regionlist)            free(out.regionlist);
			    if (out.edgelist)              free(out.edgelist);
			    if (out.edgemarkerlist)        free(out.edgemarkerlist);
			    if (out.normlist)              free(out.normlist);

            }
        }

        void take_triangulation( std::vector< point > & vp, std::vector< trifacet > & vt, std::vector<edge> & ve, facet & f)
        {

            using std::vector;
            using std::find;

            add_edges_by_points(ve);

            vector<point> main_points;
            for ( unsigned int i = 0; i < edges.size(); i++ )
            {
                int e = edges.at(i);
                point sp = vp.at(ve.at(e).start_point);
                point fp = vp.at(ve.at(e).finish_point);
                if (find(main_points.begin(), main_points.end(), sp) == main_points.end())
                    main_points.push_back(sp);
                if (find(main_points.begin(), main_points.end(), fp) == main_points.end())
                    main_points.push_back(fp);
            }

            vector<point> f_points;
            for ( unsigned int i = 0; i < f.edges.size(); i++ )
            {
                int e = f.edges.at(i);
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
            for ( unsigned int i = 0; i < main_points.size(); i++ )
            {
                dif = dif + main_points.at(i);
            }
            for ( unsigned int i = 0; i < f_points.size(); i++ )
            {
                dif = dif + f_points.at(i);
            }
            dif = dif / dif.norm();

            for (std::vector<int>::size_type i = 0; i < f.edges.size(); i++)
            {
                for ( unsigned int j = 0; j < ve.at(f.edges.at(i)).points.size(); j++ )
                {
                    int pe = ve.at(f.edges.at(i)).points.at(j);
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
            for (int i = 0; i < f.points.size(); i++)
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
            for (int i = 0; i < f.trifacets.size(); i++)
            {
                vt.push_back(trifacet(points[f.full_trifacets[i].points[0]],
                                      points[f.full_trifacets[i].points[1]],
                                      points[f.full_trifacets[i].points[2]]));
                //vt.push_back(trifacet(points[f.out.trianglelist[3*i]], points[f.out.trianglelist[3*i + 1]], points[f.out.trianglelist[3*i + 2]]));
                trifacets.push_back(vt.size()-1);
            }


        }

        point project(point p0, point dif, point normal, point m0)
        {
            return p0 + dif * (m0 - p0).dot(normal) / dif.dot(normal);
        }
    };
}
