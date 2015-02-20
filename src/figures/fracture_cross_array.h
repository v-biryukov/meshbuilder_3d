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
#include "profile/Profile.h"

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

        fracture_cross_array(std::string path, double av_step_t, REAL (*constraints_t)(REAL, REAL, REAL) = 0)
        {
            av_step = av_step_t;
            constraints = constraints_t;
            read_from_file(path);
            set_data();
        }
        virtual void read_from_file(std::string path);
        virtual void set_data();
        virtual void set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount);
        point sec_point(int i, int j, int z);
        /*
        void set_half_fracture(point start, point norm_xy, REAL length);
        void set_cross(point start, point norm_xy);
        void set_cross_part(point start, point norm_xy);
        void set_intermediate_fracture(point start, point norm_xy, REAL length);
        void set_data_part (std::vector<point>  ps, std::vector<std::vector<int> >  fs, std::vector<std::vector<int> > cs = std::vector<std::vector<int> >(0));
        int add_point(point p);
        */
    };


    void fracture_cross_array::read_from_file(std::string path)
    {
        using std::string;
        using std::cout;
        using std::cin;
        using std::endl;
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
        height = ini.request<REAL>("FCA", "height", -1);
        thickness = ini.request<REAL>("FCA", "thickness", -1);
        cross_size = ini.request<REAL>("FCA", "cross_size", -1);
        hpart = ini.request<REAL>("FCA", "hpart", -1);
        lpart = ini.request<REAL>("FCA", "lpart", -1);

        std::istringstream isx( ini.request<string>("FCA", "sections_x", "none"));
        sections_x = std::vector<REAL>( std::istream_iterator<REAL>(isx), std::istream_iterator<REAL>() );
        std::istringstream isy( ini.request<string>("FCA", "sections_y", "none"));
        sections_y = std::vector<REAL>( std::istream_iterator<REAL>(isy), std::istream_iterator<REAL>() );
        string s = ini.request<string>("FCA", "is_contact", "none");
        if ( s == "none" )
        {
            cout << "Error while reading ini file! (fracture_cross_array)";
        }
        is_contact = (s == "true" || s == "True" || s == "TRUE");
    }

    point fracture_cross_array::sec_point(int i, int j, int z)
    {
        switch (z)
        {
            case 0 : return point(sections_x.at(i), sections_y.at(j), -height/2); break;
            case 1 : return point(sections_x.at(i), sections_y.at(j), -hpart * height/2); break;
            case 2 : return point(sections_x.at(i), sections_y.at(j), +hpart * height/2); break;
            case 3 : return point(sections_x.at(i), sections_y.at(j), +height/2); break;
            default : std::cout << "Error in cross fracture array!\n"; std::exit(1);
        }
    }

    void fracture_cross_array::set_data()
    {
        point thv = point(0, thickness/2, 0);
        point thh = point(thickness/2, 0, 0);
        for (unsigned int j = 1; j < sections_y.size()-1; j++ )
            for (unsigned int i = 1; i < sections_x.size()-1; i++ )
            {
                points.push_back(sec_point(i,j,0));
                points.push_back(sec_point(i,j,1) - thv - thh);
                points.push_back(sec_point(i,j,1) + thv - thh);
                points.push_back(sec_point(i,j,1) + thv + thh);
                points.push_back(sec_point(i,j,1) - thv + thh);
                points.push_back(sec_point(i,j,2) - thv - thh);
                points.push_back(sec_point(i,j,2) + thv - thh);
                points.push_back(sec_point(i,j,2) + thv + thh);
                points.push_back(sec_point(i,j,2) - thv + thh);
                points.push_back(sec_point(i,j,3));
            }
        int n = 10 * (sections_x.size()-2);
        for ( unsigned int i = 0; i < sections_x.size()-2; i++ )
            for ( unsigned int j = 0; j < sections_y.size()-2; j++ )
            {
                int of1 = n*j + 10*i;
                int of2 = n*j + 10*(i+1);
                int of3 = n*(j+1) + 10*i;
                if ( i < sections_x.size()- 3 )
                {
                    facets.push_back(facet(of1 + 0, of1 + 3, of2 + 2, of2 + 0));
                    facets.push_back(facet(of1 + 0, of1 + 4, of2 + 1, of2 + 0));
                    facets.push_back(facet(of1 + 3, of1 + 7, of2 + 6, of2 + 2));
                    facets.push_back(facet(of1 + 4, of1 + 8, of2 + 5, of2 + 1));
                    facets.push_back(facet(of1 + 7, of1 + 9, of2 + 9, of2 + 6));
                    facets.push_back(facet(of1 + 8, of1 + 9, of2 + 9, of2 + 5));
                }
                if ( j < sections_y.size()- 3 )
                {
                    facets.push_back(facet(of1 + 0, of1 + 3, of3 + 4, of3 + 0));
                    facets.push_back(facet(of1 + 0, of1 + 2, of3 + 1, of3 + 0));
                    facets.push_back(facet(of1 + 3, of1 + 7, of3 + 8, of3 + 4));
                    facets.push_back(facet(of1 + 2, of1 + 6, of3 + 5, of3 + 1));
                    facets.push_back(facet(of1 + 7, of1 + 9, of3 + 9, of3 + 8));
                    facets.push_back(facet(of1 + 6, of1 + 9, of3 + 9, of3 + 5));
                }
            }

        point l = point(0, sections_y.at(0) - sections_y.at(1), 0)*(1-lpart);
        for ( unsigned int i = 1; i < sections_x.size()-1; i++ )
        {
            int pn = points.size();
            points.push_back(sec_point(i,0,0));
            points.push_back(sec_point(i,0,1) - l + thh);
            points.push_back(sec_point(i,0,1) - l - thh);
            points.push_back(sec_point(i,0,2) - l + thh);
            points.push_back(sec_point(i,0,2) - l - thh);
            points.push_back(sec_point(i,0,3));
            int of1 = 10*(i-1);
            facets.push_back(facet(of1 + 0, of1 + 4, pn + 1, pn));
            facets.push_back(facet(of1 + 0, of1 + 1, pn + 2, pn));
            facets.push_back(facet(of1 + 4, of1 + 8, pn + 3, pn + 1));
            facets.push_back(facet(of1 + 1, of1 + 5, pn + 4, pn + 2));
            facets.push_back(facet(of1 + 8, of1 + 9, pn + 5, pn + 3));
            facets.push_back(facet(of1 + 5, of1 + 9, pn + 5, pn + 4));
            facets.push_back(facet(pn + 0, pn + 1, pn + 3, pn + 5));
            facets.push_back(facet(pn + 0, pn + 2, pn + 4, pn + 5));
        }

        l = point(0, sections_y.at(sections_y.size()-1) - sections_y.at(sections_y.size()-2), 0)*(1-lpart);
        for ( unsigned int i = 1; i < sections_x.size()-1; i++ )
        {
            int pn = points.size();
            points.push_back(sec_point(i,sections_y.size()-1,0));
            points.push_back(sec_point(i,sections_y.size()-1,1) - l + thh);
            points.push_back(sec_point(i,sections_y.size()-1,1) - l - thh);
            points.push_back(sec_point(i,sections_y.size()-1,2) - l + thh);
            points.push_back(sec_point(i,sections_y.size()-1,2) - l - thh);
            points.push_back(sec_point(i,sections_y.size()-1,3));
            int of1 = n*(sections_y.size()-3) + 10*(i-1);
            facets.push_back(facet(of1 + 0, of1 + 3, pn + 1, pn));
            facets.push_back(facet(of1 + 0, of1 + 2, pn + 2, pn));
            facets.push_back(facet(of1 + 3, of1 + 7, pn + 3, pn + 1));
            facets.push_back(facet(of1 + 2, of1 + 6, pn + 4, pn + 2));
            facets.push_back(facet(of1 + 7, of1 + 9, pn + 5, pn + 3));
            facets.push_back(facet(of1 + 6, of1 + 9, pn + 5, pn + 4));
            facets.push_back(facet(pn + 0, pn + 1, pn + 3, pn + 5));
            facets.push_back(facet(pn + 0, pn + 2, pn + 4, pn + 5));
        }

        l = point(sections_x.at(0) - sections_x.at(1), 0, 0)*(1-lpart);
        for ( unsigned int j = 1; j < sections_y.size()-1; j++ )
        {
            int pn = points.size();
            points.push_back(sec_point(0,j,0));
            points.push_back(sec_point(0,j,1) - l + thv);
            points.push_back(sec_point(0,j,1) - l - thv);
            points.push_back(sec_point(0,j,2) - l + thv);
            points.push_back(sec_point(0,j,2) - l - thv);
            points.push_back(sec_point(0,j,3));
            int of1 = n*(j-1);
            facets.push_back(facet(of1 + 0, of1 + 2, pn + 1, pn));
            facets.push_back(facet(of1 + 0, of1 + 1, pn + 2, pn));
            facets.push_back(facet(of1 + 2, of1 + 6, pn + 3, pn + 1));
            facets.push_back(facet(of1 + 1, of1 + 5, pn + 4, pn + 2));
            facets.push_back(facet(of1 + 6, of1 + 9, pn + 5, pn + 3));
            facets.push_back(facet(of1 + 5, of1 + 9, pn + 5, pn + 4));
            facets.push_back(facet(pn + 0, pn + 1, pn + 3, pn + 5));
            facets.push_back(facet(pn + 0, pn + 2, pn + 4, pn + 5));
        }


        l = point(sections_x.at(sections_x.size()-1) - sections_x.at(sections_x.size()-2), 0, 0)*(1-lpart);
        for ( unsigned int j = 1; j < sections_y.size()-1; j++ )
        {
            int pn = points.size();
            points.push_back(sec_point(sections_x.size()-1, j, 0));
            points.push_back(sec_point(sections_x.size()-1, j, 1) - l + thv);
            points.push_back(sec_point(sections_x.size()-1, j, 1) - l - thv);
            points.push_back(sec_point(sections_x.size()-1, j, 2) - l + thv);
            points.push_back(sec_point(sections_x.size()-1, j, 2) - l - thv);
            points.push_back(sec_point(sections_x.size()-1, j, 3));
            int of1 = n*(j-1) + 10*(sections_x.size()-3);
            facets.push_back(facet(of1 + 0, of1 + 3, pn + 1, pn));
            facets.push_back(facet(of1 + 0, of1 + 4, pn + 2, pn));
            facets.push_back(facet(of1 + 3, of1 + 7, pn + 3, pn + 1));
            facets.push_back(facet(of1 + 4, of1 + 8, pn + 4, pn + 2));
            facets.push_back(facet(of1 + 7, of1 + 9, pn + 5, pn + 3));
            facets.push_back(facet(of1 + 8, of1 + 9, pn + 5, pn + 4));
            facets.push_back(facet(pn + 0, pn + 1, pn + 3, pn + 5));
            facets.push_back(facet(pn + 0, pn + 2, pn + 4, pn + 5));
        }

        if (is_contact)
        {
            std::vector<int> temp(2);
            for ( unsigned int i = 0; i < facets.size()/2; i++)
            {
                temp[0] = 2*i;
                temp[1] = 2*i+1;
                contacts.push_back(temp);
            }
        }
        hole.x = sections_x[1];
        hole.y = sections_y[1];
        hole.z = 0;
    }


    void fracture_cross_array::set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount)
    {

    }

/*
    int fracture_cross_array::add_point(point p)
    {
        std::vector<point>::iterator it = std::find(points.begin(), points.end(), p);
        if (it == points.end())
        {
            points.push_back(p);
            return points.size()-1;
        }
        else return std::distance(points.begin(), it);
    }

    void fracture_cross_array::set_data_part (std::vector<point>  ps, std::vector<std::vector<int> >  fs, std::vector<std::vector<int> > cs)
    {
        if (is_contact){
            for (int i = 0; i < cs.size(); i++)
            {
                cs.at(i).at(0) += facets.size();
                cs.at(i).at(1) += facets.size();
            }
            contacts.insert(contacts.end(), cs.begin(), cs.end());
        }
        std::vector<int> offsets_p;
        for (unsigned int i = 0; i < ps.size(); i++)
            offsets_p.push_back(add_point(ps.at(i)));
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
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * thickness/2 + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * length*lpart + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * length*lpart + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * thickness/2 + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * thickness/2 - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * length*lpart - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * length*lpart - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * thickness/2 - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);

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
            contacts_here.push_back(temp);
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
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * thickness/2 + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * (length - thickness/2) + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * (length - thickness/2) + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * thickness/2 + point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * thickness/2 - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, -height*hpart/2) + norm_xy * (length - thickness/2) - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * (length - thickness/2) - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);
        points_here.push_back(start + point(0, 0, +height*hpart/2) + norm_xy * thickness/2 - point(norm_xy.vec(point(0, 0, 1))) * thickness/2);

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
            contacts_here.push_back(temp);
            temp.clear();
        }
        set_data_part(points_here, facets_here, contacts_here);
    }

    void fracture_cross_array::set_data()
    {
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

        BOOST_FOREACH(std::vector<int> c, contacts)
            std::cout << c.at(0) << " " << c.at(1) << std::endl;

        BOOST_FOREACH(facet f, facets)
        {
            BOOST_FOREACH(int fi, f.points)
                std::cout << fi << " ";
            std::cout << std::endl;
        }

        hole.x = sections_x[1];
        hole.y = sections_y[1];
        hole.z = 0;
    }

*/
}
