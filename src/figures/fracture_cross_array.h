/*****************************************************************************
* name: fracture_cross_array.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Derived class from figure. Represents an array of geological intersecting fractures.
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
    struct fracture_cross_array : public figure
    {
        bool is_contact;
        REAL height;
        REAL thickness;
        REAL hpart, lpart;
        REAL cross_size;
        std::vector<REAL> sections_x;
        std::vector<REAL> sections_y;
        std::vector<REAL> angles_x;
        std::vector<REAL> angles_y;

        fracture_cross_array(std::string path, double av_step_t, REAL (*constraints_t)(REAL, REAL, REAL) = 0)
        {
            av_step = av_step_t;
            constraints = constraints_t;
            read_from_file(path);
            set_data();
        }
        virtual void read_from_file(std::string path);
        void set_half_fracture(point start, point norm_xy, REAL length);
        void set_cross(point start, point norm_xy);
        void set_cross_part(point start, point norm_xy);
        void set_intermediate_fracture(point start, point norm_xy, REAL length);
        void set_data_part (std::vector<point>  ps, std::vector<std::vector<int> >  fs, std::vector<std::vector<int> > cs = std::vector<std::vector<int> >(0));
        virtual void set_data();

        int add_point(point p, bool check);
    };


    void fracture_cross_array::read_from_file(std::string path)
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
        height = pt.get<REAL>("FCA.height");
        thickness = pt.get<REAL>("FCA.thickness");
        cross_size = pt.get<REAL>("FCA.cross_size");
        hpart = pt.get<REAL>("FCA.hpart");
        lpart = pt.get<REAL>("FCA.lpart");
        std::istringstream isx( pt.get<string>("FCA.sections_x"));
        sections_x = std::vector<REAL>( std::istream_iterator<REAL>(isx), std::istream_iterator<REAL>() );
        std::istringstream isy( pt.get<string>("FCA.sections_y"));
        sections_y = std::vector<REAL>( std::istream_iterator<REAL>(isy), std::istream_iterator<REAL>() );
        string s = pt.get<string>("FCA.is_contact");
        if (s == "true" || s == "True" || s == "TRUE")
            is_contact = true;
        else
            is_contact = false;
    }

    int fracture_cross_array::add_point(point p, bool check)
    {
        if (check)
        {
            std::vector<point>::iterator it = std::find(points.begin(), points.end(), p);
            if (it == points.end())
            {
                points.push_back(p);
                return points.size()-1;
            }
            else return std::distance(points.begin(), it);
        }
        else
        {
            points.push_back(p);
            return points.size()-1;
        }
    }

    void fracture_cross_array::set_data_part (std::vector<point>  ps, std::vector<std::vector<int> >  fs, std::vector<std::vector<int> > cs)
    {
        if (is_contact){
            //BOOST_FOREACH (std::vector<int> & c, cs){
            //    c.at(0) += facets.size();
            //    c.at(1) += facets.size();
            //}
            for (int i = 0; i < cs.size(); i++)
            {
                cs.at(i).at(0) += facets.size();
                cs.at(i).at(0) += facets.size();
            }
            contacts.insert(contacts.end(), cs.begin(), cs.end());
        }
        std::vector<int> offsets_p;
        for (unsigned int i = 0; i < ps.size(); i++)
            offsets_p.push_back(add_point(ps.at(i), true));
        for (unsigned int i = 0; i < fs.size(); i++)
            if (fs.at(i).size() == 3)
                facets.push_back(facet(offsets_p.at(fs.at(i).at(0)), offsets_p.at(fs.at(i).at(1)), offsets_p.at(fs.at(i).at(2))));
            else if (fs.at(i).size() == 4)
                facets.push_back(facet(offsets_p.at(fs.at(i).at(0)), offsets_p.at(fs.at(i).at(1)), offsets_p.at(fs.at(i).at(2)), offsets_p.at(fs.at(i).at(3))));

    }

    void fracture_cross_array::set_half_fracture(point start, point norm_xy, REAL length)
    {
        // lplane --- length*lpart on the this part of fracture
        norm_xy = norm_xy - point(0, 0, 1) * norm_xy.dot(point(0, 0, 1));
        norm_xy = norm_xy / norm_xy.norm();
        //point th = point(norm_xy.vec(point(0, 0, 1)));
        std::vector<point> points_here;
        points_here.push_back(start + point(0, 0, -height/2));
        points_here.push_back(start + point(0, 0, -height/2) + norm_xy * length);
        points_here.push_back(start + point(0, 0, +height/2) + norm_xy * length);
        points_here.push_back(start + point(0, 0, +height/2));
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * cross_size + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * length*lpart + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * length*lpart + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * cross_size + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * cross_size - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * length*lpart - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * length*lpart - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * cross_size - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);

        //std::vector<std::vector<int>> edges_here = {{0, 1}, {1, 2}, {2, 3}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {8, 9}, {9, 10}, {10, 11}, {11, 8},
        //                                            {0, 4}, {1, 5}, {2, 6}, {3, 7}, {0, 8}, {1, 9}, {2, 10}, {3, 11}};

        std::vector<std::vector<int> > facets_here;
        int temp_arr[8][4] = {{0, 1, 5, 4}, {0, 1, 9, 8}, {1, 2, 6, 5}, {1, 2, 10, 9}, {2, 3, 7, 6}, {2, 3, 11, 10}, {4, 5, 6, 7}, {8, 9, 10, 11}};
        std::vector<int> temp;
        for(int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 4; j++)
                temp.push_back(temp_arr[i][j]);
            facets_here.push_back(temp);
            temp.clear();
        }

        std::vector<std::vector<int> > contacts_here;
        int temp_arr_c[4][2] = {{0, 1}, {2, 3}, {4, 5}, {6, 7}};
        for(int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 2; j++)
                temp.push_back(temp_arr_c[i][j]);
            facets_here.push_back(temp);
            temp.clear();
        }
        set_data_part(points_here, facets_here, contacts_here);
    }
/*
    void fracture_cross_array::set_cross_part(point start, point norm_xy)
    {
        norm_xy = norm_xy - point(0, 0, 1) * norm_xy.dot(point(0, 0, 1));
        norm_xy = norm_xy / norm_xy.norm();
        std::vector<point> points_here = {start + point(0, 0, -height/2),
                                          start + point(0, 0, -height*hpart/2) + norm_xy * cross_size + point(norm_xy.vec(point(0, 0, 1))) * thickness/2,
                                          start + point(0, 0, -height*hpart/2) + point(norm_xy.vec(point(0, 0, 1))) * cross_size + norm_xy * thickness/2,
                                          start + point(0, 0, +height*hpart/2) + norm_xy * cross_size + point(norm_xy.vec(point(0, 0, 1))) * thickness/2,
                                          start + point(0, 0, +height*hpart/2) + point(norm_xy.vec(point(0, 0, 1))) * cross_size + norm_xy * thickness/2,
                                          start + point(0, 0, +height/2),
                                          start + point(0, 0, -height*hpart/2) - norm_xy * cross_size - point(norm_xy.vec(point(0, 0, 1))) * thickness/2,
                                          start + point(0, 0, -height*hpart/2) - point(norm_xy.vec(point(0, 0, 1))) * cross_size - norm_xy * thickness/2,
                                          start + point(0, 0, +height*hpart/2) - norm_xy * cross_size - point(norm_xy.vec(point(0, 0, 1))) * thickness/2,
                                          start + point(0, 0, +height*hpart/2) - point(norm_xy.vec(point(0, 0, 1))) * cross_size - norm_xy * thickness/2,
                                          };
        //std::vector<std::vector<int>> edges_here = {{0, 1}, {0, 2}, {1, 3}, {2, 4}, {3, 5}, {4, 5}, {1, 2}, {3, 4}, {0, 6}, {0, 7}, {6, 8}, {7, 9}, {8, 5}, {9, 5}, {6, 7}, {8, 9}};
        std::vector<std::vector<int>> facets_here = {{0, 1, 2}, {0, 6, 7}, {2, 1, 3, 4}, {7, 6, 8, 9}, {4, 3, 5}, {9, 8, 5}};
        std::vector<std::vector<int>> contacts_here = {{0, 1}, {2, 3}, {4, 5}};
        set_data_part(points_here, facets_here, contacts_here);
    }
/*
    void fracture_cross_array::set_cross(point start, point norm_xy)
    {
        set_cross_part(start, norm_xy);
        norm_xy = norm_xy.vec(point(0, 0, 1));
        set_cross_part(start, norm_xy);
    }*/
    void fracture_cross_array::set_cross(point start, point norm_xy)
    {
        norm_xy = norm_xy - point(0, 0, 1) * norm_xy.dot(point(0, 0, 1));
        norm_xy = norm_xy / norm_xy.norm();
        point perp_xy = norm_xy.vec(point(0, 0, 1));
        std::vector<point> points_here;
        points_here.push_back(start + point(0, 0, -height/2));
        points_here.push_back(start + point(0, 0, +height/2));
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * cross_size + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + point(norm_xy.vec(point(0, 0, 1))) * cross_size + norm_xy * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * cross_size + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + point(norm_xy.vec(point(0, 0, 1))) * cross_size + norm_xy * thickness/2);

        points_here.push_back(start + point(0, 0, -height*hpart/2) - point(norm_xy.vec(point(0, 0, 1))) * cross_size - norm_xy * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) - norm_xy * cross_size - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) - point(norm_xy.vec(point(0, 0, 1))) * cross_size - norm_xy * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) - norm_xy * cross_size - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);

        points_here.push_back(start + point(0, 0, -height*hpart/2) + point(perp_xy.vec(point(0, 0, 1))) * cross_size + perp_xy * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + perp_xy * cross_size + point(perp_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + point(perp_xy.vec(point(0, 0, 1))) * cross_size + perp_xy * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + perp_xy * cross_size + point(perp_xy.vec(point(0, 0, 1))) * thickness/2);

        points_here.push_back(start + point(0, 0, -height*hpart/2) - perp_xy * cross_size - point(perp_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) - point(perp_xy.vec(point(0, 0, 1))) * cross_size - perp_xy * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) - perp_xy * cross_size - point(perp_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) - point(perp_xy.vec(point(0, 0, 1))) * cross_size - perp_xy * thickness/2);

        std::vector<std::vector<int> > facets_here;
        int temp_arr[12][4] = {{0, 2, 3, -1}, {0, 10, 11, -1}, {2, 3, 5, 4}, {10, 11, 13, 12}, {5, 4, 1, -1}, {13, 12, 1, -1},{0, 6, 7, -1}, {0, 14, 15, -1}, {6, 7, 9, 8},
                           {14, 15, 17, 16}, {9, 8, 1, -1}, {17, 16, 1, -1}};

        std::vector<int> temp;
        for(int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 4; j++)
                if (temp_arr[i][j] >= 0)
                    temp.push_back(temp_arr[i][j]);

            facets_here.push_back(temp);
            temp.clear();
        }

        std::vector<std::vector<int> > contacts_here;
        int temp_arr_c[6][2] = {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}, {10, 11}};
        for(int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 2; j++)
                temp.push_back(temp_arr_c[i][j]);
            facets_here.push_back(temp);
            temp.clear();
        }
        set_data_part(points_here, facets_here, contacts_here);
    }
    void fracture_cross_array::set_intermediate_fracture(point start, point norm_xy, REAL length)
    {
        norm_xy = norm_xy - point(0, 0, 1) * norm_xy.dot(point(0, 0, 1));
        norm_xy = norm_xy / norm_xy.norm();
        //point th = point(norm_xy.vec(point(0, 0, 1)));

        std::vector<point> points_here;
        points_here.push_back(start + point(0, 0, -height/2));
        points_here.push_back(start + point(0, 0, -height/2) + norm_xy * length);
        points_here.push_back(start + point(0, 0, +height/2) + norm_xy * length);
        points_here.push_back(start + point(0, 0, +height/2));
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * cross_size + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * (length - cross_size) + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * (length - cross_size) + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * cross_size + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * cross_size - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * (length - cross_size) - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * (length - cross_size) - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * cross_size - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);

        std::vector<std::vector<int> > facets_here;
        int temp_arr[6][4] = {{0, 1, 5, 4}, {0, 1, 9, 8}, {2, 3, 7, 6}, {2, 3, 11, 10}, {4, 5, 6, 7}, {8, 9, 10, 11}};
        std::vector<int> temp;
        for(int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 4; j++)
                temp.push_back(temp_arr[i][j]);
            facets_here.push_back(temp);
            temp.clear();
        }

        std::vector<std::vector<int> > contacts_here;
        int temp_arr_c[3][2] = {{0, 1}, {2, 3}, {4, 5}};
        for(int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 2; j++)
                temp.push_back(temp_arr_c[i][j]);
            facets_here.push_back(temp);
            temp.clear();
        }
        set_data_part(points_here, facets_here, contacts_here);
    }

    void fracture_cross_array::set_data()
    {

        for (std::vector<REAL>::size_type i = 1; i != sections_x.size()-1; i++)
                for (std::vector<REAL>::size_type j = 1; j != sections_y.size()-1; j++)
                    set_cross(point(sections_x[i], sections_y[j], 0), point(1, 0, 0));

        //cross_size = thickness/2;
        for (std::vector<REAL>::size_type j = 1; j != sections_y.size()-1; j++)
        {
            set_half_fracture(point(sections_x[1], sections_y[j], 0), point(-1, 0, 0), sections_x[1]-sections_x[0]);
            set_half_fracture(point(sections_x[sections_x.size()-2], sections_y[j], 0), point(1,  0, 0), sections_x[sections_x.size()-1]-sections_x[sections_x.size()-2]);
            for (std::vector<REAL>::size_type i = 1; i != sections_x.size()-2; i++)
                    set_intermediate_fracture(point(sections_x[i], sections_y[j], 0), point(1, 0, 0), sections_x[i+1] - sections_x[i]);
        }


        for (std::vector<REAL>::size_type i = 1; i != sections_x.size()-1; i++)
        {
            set_half_fracture(point(sections_x[i], sections_y[1], 0), point(0, -1, 0), sections_y[1]-sections_y[0]);
            set_half_fracture(point(sections_x[i], sections_y[sections_y.size()-2], 0), point(0,  1, 0), sections_y[sections_y.size()-1]-sections_y[sections_y.size()-2]);

            for (std::vector<REAL>::size_type j = 1; j != sections_y.size()-2; j++)
                set_intermediate_fracture(point(sections_x[i], sections_y[j], 0), point(0, 1, 0), sections_y[j+1] - sections_y[j]);
        }

        hole.x = sections_x[1];
        hole.y = sections_y[1];
        hole.z = 0;
    }


}
