/*****************************************************************************
* name: cube.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Derived class from figure. Represents a cube.
*
* license: GPLv3
*
*****************************************************************************/

#pragma once
#include "../figure.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace swift
{
    struct cube : public figure
    {
        REAL size;

        cube(std::string path, double av_step_t, REAL (*constraints_t)(REAL, REAL, REAL) = 0)
        {
            av_step = av_step_t;
            constraints = constraints_t;
            read_from_file(path);
            set_data();
        }
        virtual void read_from_file(std::string path);
        virtual void set_data();
    };


    void cube::read_from_file(std::string path)
    {
        using std::string;
        using std::cout;
        using std::cin;
        using std::endl;
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
            std::cin.get();
            // need to quit from the program
        }
        size = pt.get<REAL>("Cube.size");
    }

    void cube::set_data()
    {
        points.push_back(point(-size/2, -size/2, -size/2));
        points.push_back(point(-size/2, -size/2, +size/2));
        points.push_back(point(-size/2, +size/2, -size/2));
        points.push_back(point(+size/2, -size/2, -size/2));
        points.push_back(point(-size/2, +size/2, +size/2));
        points.push_back(point(+size/2, -size/2, +size/2));
        points.push_back(point(+size/2, +size/2, -size/2));
        points.push_back(point(+size/2, +size/2, +size/2));
        facets.push_back(facet(1, 5, 7, 4));
        facets.push_back(facet(0, 3, 6, 2));
        facets.push_back(facet(0, 1, 5, 3));
        facets.push_back(facet(0, 2, 4, 1));
        facets.push_back(facet(7, 5, 3, 6));
        facets.push_back(facet(6, 7, 4, 2));
    }

}
