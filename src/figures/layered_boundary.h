/*****************************************************************************
* name: layered_boundary.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Derived class from figure. Represents a geological cube with layered contact boundaries.
*
* license: GPLv3
*
*****************************************************************************/

#pragma once

#include "../figure.h"
#include "../facet.h"
#include <sstream>
#include "../profile/Profile.h"
#include <fstream>
#include <limits>
#include <map>

namespace swift
{
    struct layered_boundary : public figure
    {

        int number_of_layers;
        REAL z0, z1;
        REAL discretization_step;
        std::string layer_path;
        std::string xy_boundary_path;

        std::vector<point> boundary_points;
        std::vector<point> xy_points;
        std::vector<trifacet> xy_trifacets;

        const REAL eps = 1e-3;
        const REAL contact_shift = 1e-3;

        layered_boundary(std::string path, double av_step_t, REAL (*constraints_t)(REAL, REAL, REAL) = 0)
        {
            av_step = av_step_t;
            read_from_file(path);
            set_data();
        }
        virtual void read_from_file(std::string path);
        virtual void set_data();
        virtual void set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount);

        void set_facets();
        void basic_divide_edges();
        void basic_triangulate();
        void read_layer(int nun_of_layer, std::vector<point> & layer);
        void set_layer_points(std::vector<point> & layer, point contact_shift);
    };

    void layered_boundary::read_from_file(std::string path)
    {
        using std::string;
        using std::cout;
        using std::cin;
        using std::endl;
        using std::istringstream;
        using std::vector;
        using std::istream_iterator;
        Profile ini;
        try
        {
            ini = Profile(path);
        }
        catch (...)
        {
            cout << "Error while reading ini file! Press any key to close." << endl;
            cin.get();
            std::exit(1);
        }
        istringstream is( ini.request<string>("Layered_boundary", "z", "none") );
        vector<REAL> t = vector<REAL>( istream_iterator<REAL>(is), istream_iterator<REAL>());
        z0 = t.at(0);
        z1 = t.at(1);
        number_of_layers = ini.request<int>("Layered_boundary", "number_of_layers", -1);
        discretization_step = ini.request<REAL>("Layered_boundary", "discretization_step", -1.0);
        layer_path = ini.request<std::string>("Layered_boundary", "layer_path", "");
        xy_boundary_path = ini.request<std::string>("Layered_boundary", "xy_boundary_path", "");

        std::ifstream ifs(xy_boundary_path.c_str());
        if (!ifs)
        {
            std::cout << "Error in reading xyboundary file" << std::endl;
            std::exit(1);
        }
        double x = 0.0, y = 0.0;
        while (ifs >> x && ifs >> y)
            boundary_points.push_back(point(x, y, 0.0));
        ifs.close();

        basic_divide_edges();
        basic_triangulate();
    }

    void layered_boundary::basic_divide_edges()
    {
        boundary_points.push_back(boundary_points[0]);
        std::vector<point> edge_points;

        for (int k = 0; k < boundary_points.size()-1; k++)
        {
            edge_points.push_back(boundary_points[k]);
            // set intermediate points
            REAL norm = (boundary_points[k+1]-boundary_points[k]).norm();
            int n = int(ceil(norm / discretization_step));
            point dif = (boundary_points[k+1]-boundary_points[k]) / n;
            for (int i = 1; i < n; i++)
            {
                point p = boundary_points[k] + dif * i;
                edge_points.push_back(p);
            }
        }
        boundary_points = edge_points;
    }

    void layered_boundary::basic_triangulate()
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

        in.numberofpoints = boundary_points.size();
        in.numberofpointattributes = 0;
        in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
        in.pointmarkerlist = (int *) NULL;
        in.pointattributelist = (REAL *) NULL;

        for (int i = 0; i < in.numberofpoints; i++)
        {
            in.pointlist[2*i + 0] = boundary_points[i].x;
            in.pointlist[2*i + 1] = boundary_points[i].y;
        }

        in.numberofsegments = in.numberofpoints;
        in.segmentlist = (int *) malloc(in.numberofsegments * 2 * sizeof(int));
        in.segmentmarkerlist = (int *) NULL;
        for (int i = 0; i < in.numberofsegments; i++)
        {
            in.segmentlist[2*i] = i;
            in.segmentlist[2*i+1] = i+1;
        }
        in.segmentlist[2*in.numberofsegments-1] = 0;

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
        ss5 << "pzqYQa" << discretization_step * discretization_step / 2;
        std::string str;
        ss5 >> str;
        char * s = new char[str.size() + 1];
        std::copy(str.begin(), str.end(), s);
        s[str.size()] = '\0';
        triangulate(s, &in, &out, (struct triangulateio *) NULL);


        for (int i = 0; i < out.numberofpoints; i++)
        {
            xy_points.push_back(point(out.pointlist[2*i], out.pointlist[2*i+1], 0.0));
        }

        for (int i = 0; i < out.numberoftriangles; i++)
        {
            xy_trifacets.push_back(trifacet(out.trianglelist[3*i], out.trianglelist[3*i+1], out.trianglelist[3*i+2]));
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

    bool ReplaceSubstring(std::string& str, const std::string& from, const std::string& to)
    {
        size_t start_pos = str.find(from);
        if(start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    void layered_boundary::read_layer(int nun_of_layer, std::vector<point> & layer)
    {
        layer.resize(0);
        std::stringstream ss;
        ss << nun_of_layer;
        std::string path = layer_path;

        if (!ReplaceSubstring(path, "<index>", ss.str()))
        {
            std::cout << "Error in reading layers data" << std::endl;
            std::exit(1);
        }
        std::ifstream ifs(path.c_str());
        if (!ifs)
        {
            std::cout << "Error in reading layers data" << std::endl;
            std::exit(1);
        }
        double x = 0.0, y = 0.0, z = 0.0;
        while (ifs >> x && ifs >> y && ifs >> z)
            layer.push_back(point(x, y, z));
        ifs.close();
    }

    struct cell
    {
        int x;
        int y;
        cell() {}
        cell(int x, int y) :x(x), y(y) {}
    };
    struct cell_compare
    {
       bool operator() (const cell& lhs, const cell& rhs)
       {
           return (lhs.x < rhs.x) || (lhs.x == rhs.x && lhs.y < rhs.y);
       }
    };

    cell get_cell(point p, int num_of_layer_sections, point min_p, point max_p)
    {
        point dif = (max_p - min_p) / num_of_layer_sections;
        cell c;
        c.x = (int)floor((p.x-min_p.x) / dif.x);
        c.y = (int)floor((p.y-min_p.y) / dif.y);
        return c;
    }

    void find_close_points(cell c, int num_of_layer_sections, std::vector<point> & close_points, std::map< cell, std::vector<point>, cell_compare > & point_map)
    {
        int rad = 0;
        while (close_points.size() < 3)
        {
            for (int i = -rad; i <= rad; i++)
                for (int j = -rad; j <= rad; j++)
                {
                    if ( c.x + i >= 0 && c.x + i < num_of_layer_sections && c.y + j >= 0 && c.y + j < num_of_layer_sections )
                    {
                        std::vector<point> temp = point_map[cell(c.x+i, c.y+j)];
                        for (int k = 0; k < temp.size(); k++)
                        {
                            if (std::find(close_points.begin(), close_points.end(), temp[k]) == close_points.end())
                                close_points.push_back(temp[k]);
                        }
                    }
                }
            rad++;
        }
    }

    void layered_boundary::set_layer_points(std::vector<point> & layer, point contact_shift)
    {
        int num_of_layer_sections = int(floor(sqrt(layer.size())));

        std::map< cell, std::vector<point>, cell_compare > point_map;

        point min_point = layer[0];
        point max_point = layer[0];
        for (int i = 1; i < layer.size(); i++)
        {
            if (layer[i].x > max_point.x) max_point.x = layer[i].x;
            if (layer[i].y > max_point.y) max_point.y = layer[i].y;
            if (layer[i].z > max_point.z) max_point.z = layer[i].z;
            if (layer[i].x < min_point.x) min_point.x = layer[i].x;
            if (layer[i].y < min_point.y) min_point.y = layer[i].y;
            if (layer[i].z < min_point.z) min_point.z = layer[i].z;
        }
        min_point = min_point - (max_point - min_point) / num_of_layer_sections;
        max_point = max_point + (max_point - min_point) / num_of_layer_sections;
        for (int i = 0; i < num_of_layer_sections; i++)
            for (int j = 0; j < num_of_layer_sections; j++)
            {
                point_map.insert(std::pair<cell, std::vector<point> >(cell(i, j), std::vector<point>()));
            }
        for (int i = 0; i < layer.size(); i++)
        {
            point_map[get_cell(layer[i], num_of_layer_sections, min_point, max_point)].push_back(layer[i]);
        }

        for (int i = 0; i < xy_points.size(); i++)
        {
            std::vector<point> close_points, closest_points;
            cell c = get_cell(xy_points[i], num_of_layer_sections, min_point, max_point);
            find_close_points(c, num_of_layer_sections, close_points, point_map);

            for (int j = 0; j < 3; j++)
            {
                point closest = close_points[0];
                int closest_index = 0;
                for (int k = 1; k < close_points.size(); k++)
                {
                    if ((closest-xy_points[i]).norm_sq() > (close_points[k]-xy_points[i]).norm_sq())
                    {
                        closest = close_points[k];
                        closest_index = k;
                    }
                }
                closest_points.push_back(closest);
                close_points.erase(close_points.begin() + closest_index);
            }



            point r = xy_points[i];
            point ra = closest_points[0];
            point rb = closest_points[1];
            point rc = closest_points[2];
            REAL sa = (rc - rb).vecz(r-rb);
            REAL sb = (ra - rc).vecz(r-rc);
            REAL sc = (rb - ra).vecz(r-ra);
            REAL s = sa + sb + sc;
            REAL z;
            if (fabs(s) < eps)
            {
                z = ra.z;
            }
            else
            {
                sa /= s; sb /= s; sc /= s;
                z = sa * ra.z + sb * rb.z + sc * rc.z;
            }

            points.push_back(point(xy_points[i].x, xy_points[i].y, z) + contact_shift);

        }

    }



    void layered_boundary::set_data()
    {
        // Setting points
        std::cout << "Setting layers data points." << std::endl;
        // TODO
        std::vector<point> layer;

        for (int i = 0; i < xy_points.size(); i++)
            points.push_back(point(xy_points[i].x, xy_points[i].y, z0));
        for (int layer_i = 0; layer_i < number_of_layers; layer_i++)
        {
            read_layer(layer_i+1, layer);



            for (int i = 0; i < xy_points.size(); i++)
            {
                double min_norm = (xy_points[0]- xy_points[1]).norm_sq();
                int min_index = 0;
                for (int j = 1; j < layer.size(); j++)
                {
                    if ((xy_points[i].x-layer[j].x) * (xy_points[i].x-layer[j].x) +
                        (xy_points[i].y-layer[j].y) * (xy_points[i].y-layer[j].y)  < min_norm)
                    {
                        min_index = j;
                    }
                }
                points.push_back(point(xy_points[i].x, xy_points[i].y, layer[min_index].z) + point(0,0,layer_i * contact_shift));
            }


            //set_layer_points(layer, point(0,0,layer_i * contact_shift));


            int current_point_size = points.size();
            for (int i = 0; i < xy_points.size(); i++)
            {
                points.push_back(points[current_point_size - xy_points.size() + i] + point(0,0, contact_shift));
            }

            std::cout << "Layer " << layer_i + 1 << " points have been set"<< std::endl;
        }
        for (int i = 0; i < xy_points.size(); i++)
            points.push_back(point(xy_points[i].x, xy_points[i].y, z1 + number_of_layers * contact_shift));

        // Setting facets:
        std::cout << "Setting layers data facets." << std::endl;
        for (int layer_i = 0; layer_i < 2 * number_of_layers + 2; layer_i++)
        {
            for (int j = 0; j < xy_trifacets.size(); j++)
            {
                facets.push_back(facet(layer_i*xy_points.size() + xy_trifacets[j].points[0],
                                       layer_i*xy_points.size() + xy_trifacets[j].points[1],
                                       layer_i*xy_points.size() + xy_trifacets[j].points[2]));
            }
        }

        // Setting contacts
        contacts.resize(xy_trifacets.size() * number_of_layers, std::vector<int>(2));
        for (int layer_i = 0; layer_i < number_of_layers; layer_i++)
        {
            for (int j = 0; j < xy_trifacets.size(); j++)
            {
                contacts[layer_i*xy_trifacets.size() + j][0] = xy_trifacets.size() * (2*layer_i + 1) + j;
                contacts[layer_i*xy_trifacets.size() + j][1] = xy_trifacets.size() * (2*layer_i + 2) + j;
            }
        }

        // Vertical borders - facets

        for (int layer_i = 0; layer_i < number_of_layers+1; layer_i++)
        {
            for (int i = 0; i < boundary_points.size()-1; i++)
            {
                facets.push_back(facet(2*layer_i*xy_points.size() + i, 2*layer_i*xy_points.size() + i+1, (2*layer_i+1)*xy_points.size() + i+1, (2*layer_i+1)*xy_points.size() + i));
            }
            facets.push_back(facet(2*layer_i*xy_points.size() + boundary_points.size()-1, 2*layer_i*xy_points.size(), (2*layer_i+1)*xy_points.size() + 0, (2*layer_i+1)*xy_points.size() + boundary_points.size()-1));
        }
    }


    void layered_boundary::set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount)
    {

        for (int i = 0; i < number_of_layers; i++)
        {
            int num_of_layer_trifacets = 0;
            for (int j = 0; j < xy_trifacets.size(); j++)
            {
                num_of_layer_trifacets += this->facets[(2*i+1)*xy_trifacets.size() + j].trifacets.size();
            }
            contactFacesCount.push_back(num_of_layer_trifacets);
        }

        // Upper border
        int num_of_border_trifacets = 0;

        for (int j = 0; j < xy_trifacets.size(); j++)
        {
            num_of_border_trifacets += this->facets[j].trifacets.size();
        }
        boundaryFacesCount.push_back(num_of_border_trifacets);

        // Bottom border
        boundaryFacesCount.push_back(num_of_border_trifacets);

        // Left Right Front Back borders
        num_of_border_trifacets = 0;
        for (int i = 0; i < number_of_layers+1; i++)
        {
            for (int j = 0; j < boundary_points.size(); j++)
            {
                num_of_border_trifacets += this->facets[(2+2*number_of_layers)*xy_trifacets.size() + boundary_points.size()*i +j].trifacets.size();
            }
        }
        boundaryFacesCount.push_back(num_of_border_trifacets);

    }

}
