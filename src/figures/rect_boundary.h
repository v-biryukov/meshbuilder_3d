/*****************************************************************************
* name: rect_boundary.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Derived class from figure. Represents a geological cube with contact boundaries.
*
* license: GPLv3
*
*****************************************************************************/

#pragma once

#include "../figure.h"
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace swift
{
    struct rect_boundary : public figure
    {
        point p1, p2;
        std::vector<REAL> slice_z;
        std::vector<point> slice_normal;
        //point eps = point(0, 0, -1e-3);

        rect_boundary(std::string path, double av_step_t, REAL (*constraints_t)(REAL, REAL, REAL) = 0)
        {
            av_step = av_step_t;
            constraints = constraints_t;
            read_from_file(path);
            set_data();
        }
        virtual void read_from_file(std::string path);
        virtual void set_data();
        void set_facets();
        point get_z_point (point normal, REAL z0, REAL x, REAL y);
    };

    void rect_boundary::read_from_file(std::string path)
    {
        using std::string;
        using std::cout;
        using std::cin;
        using std::endl;
        using std::istringstream;
        using std::vector;
        using std::istream_iterator;
        using boost::lexical_cast;
        boost::property_tree::ptree pt;
        try
        {
            boost::property_tree::read_ini(path, pt);
        }
        catch (boost::property_tree::ini_parser_error& error)
        {
            cout
                << error.message() << ": "
                << error.filename() << ", line "
                << error.line() << endl;
            cout << "Error! Press any key to close." << endl;
            cin.get();
            // need to quit from the program
        }
        istringstream is( pt.get<string>("Rect_boundary.x") );
        vector<REAL> t = vector<REAL>( istream_iterator<REAL>(is), istream_iterator<REAL>());
        p1.x = t.at(0);
        p2.x = t.at(1);
        is.clear();
        is.str( pt.get<string>("Rect_boundary.y") );
        t = vector<REAL>( istream_iterator<REAL>(is), istream_iterator<REAL>());
        p1.y = t.at(0);
        p2.y = t.at(1);
        is.clear();
        is.str( pt.get<string>("Rect_boundary.z") );
        t = vector<REAL>( istream_iterator<REAL>(is), istream_iterator<REAL>());
        p1.z = t.at(0);
        p2.z = t.at(1);
        int slice_num = pt.get<int>("Rect_boundary.number_of_slices");
        for (int i = 1; i <= slice_num; i++)
        {
            slice_z.push_back(pt.get<REAL>("Rect_boundary.slice" + lexical_cast<string>(i) + "_z"));
            istringstream is( pt.get<string>("Rect_boundary.slice" + lexical_cast<string>(i) + "_normal") );
            t = vector<REAL>( istream_iterator<REAL>(is), istream_iterator<REAL>());
            point p (t.at(0), t.at(1), t.at(2));
            p = p / p.norm();
            slice_normal.push_back(p);
        }
    }

    point rect_boundary::get_z_point (point normal, REAL z0, REAL x, REAL y)
    {
        return point(x, y, z0 + normal.x * (p1.x/2 + p2.x/2 - x) + normal.y * (p1.y/2 + p2.y/2 - y));
    }

    void rect_boundary::set_facets()
    {
        // Top
        facets.push_back(facet(0, 1, 2, 3));

        // Bottom
        facets.push_back(facet(4 + 8*slice_z.size(), 5 + 8*slice_z.size(), 6 + 8*slice_z.size(), 7 + 8*slice_z.size()));
        // Front
        for (unsigned int i = 0; i < slice_z.size(); i++)
            facets.push_back(facet(0 + 8*i, 4 + 8*i, 5 + 8*i, 1 + 8*i));
        facets.push_back(facet(0 + 8*slice_z.size(), 4 + 8*slice_z.size(), 5 + 8*slice_z.size(), 1 + 8*slice_z.size()));
        // Left

        for (unsigned int i = 0; i < slice_z.size(); i++)
            facets.push_back(facet(1 + 8*i, 5 + 8*i, 6 + 8*i, 2 + 8*i));
        facets.push_back(facet(1 + 8*slice_z.size(), 5 + 8*slice_z.size(), 6 + 8*slice_z.size(), 2 + 8*slice_z.size()));
        // Back
        for (unsigned int i = 0; i < slice_z.size(); i++)
            facets.push_back(facet(2 + 8*i, 6 + 8*i, 7 + 8*i, 3 + 8*i));
        facets.push_back(facet(2 + 8*slice_z.size(), 6 + 8*slice_z.size(), 7 + 8*slice_z.size(), 3 + 8*slice_z.size()));
        // Right
        for (unsigned int i = 0; i < slice_z.size(); i++)
            facets.push_back(facet(3 + 8*i, 7 + 8*i, 4 + 8*i, 0 + 8*i));
        facets.push_back(facet(3 + 8*slice_z.size(), 7 + 8*slice_z.size(), 4 + 8*slice_z.size(), 0 + 8*slice_z.size()));
        // Contacts

        for (unsigned int i = 0; i < slice_z.size(); i++)
        {
            facets.push_back(facet(4 + 8*i, 5 + 8*i, 6 + 8*i, 7 + 8*i));
            facets.push_back(facet(8 + 8*i, 9 + 8*i, 10 + 8*i, 11 + 8*i));
            std::vector<int> temp;
            temp.push_back(facets.size() - 2);
            temp.push_back(facets.size() - 1);
            contacts.push_back(temp);
        }
    }

    void rect_boundary::set_data()
    {
        points.push_back(point(p2.x, p1.y, p2.z));
        points.push_back(point(p1.x, p1.y, p2.z));
        points.push_back(point(p1.x, p2.y, p2.z));
        points.push_back(point(p2.x, p2.y, p2.z));
        for (unsigned int i = 0; i < slice_z.size(); i++)
        {
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p2.x, p1.y) + point(0, 0, -1e-3)*i);
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p1.x, p1.y) + point(0, 0, -1e-3)*i);
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p1.x, p2.y) + point(0, 0, -1e-3)*i);
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p2.x, p2.y) + point(0, 0, -1e-3)*i);
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p2.x, p1.y) + point(0, 0, -1e-3)*(i+1));
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p1.x, p1.y) + point(0, 0, -1e-3)*(i+1));
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p1.x, p2.y) + point(0, 0, -1e-3)*(i+1));
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p2.x, p2.y) + point(0, 0, -1e-3)*(i+1));
        }
        points.push_back(point(p2.x, p1.y, p1.z));
        points.push_back(point(p1.x, p1.y, p1.z));
        points.push_back(point(p1.x, p2.y, p1.z));
        points.push_back(point(p2.x, p2.y, p1.z));

        set_facets();
    }

}
