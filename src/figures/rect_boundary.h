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
#include "../profile/Profile.h"

namespace swift
{
    struct rect_boundary : public figure
    {
        point p1, p2;
        std::vector<REAL> slice_z;
        std::vector<point> slice_normal;
        const REAL contact_shift = 5000.0;//1e-3;
        bool is_continuous;
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
        virtual void set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount);
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
        istringstream is( ini.request<string>("Rect_boundary", "x", "none") );
        vector<REAL> t = vector<REAL>( istream_iterator<REAL>(is), istream_iterator<REAL>());
        p1.x = t.at(0);
        p2.x = t.at(1);
        is.clear();
        is.str( ini.request<string>("Rect_boundary", "y", "none") );
        t = vector<REAL>( istream_iterator<REAL>(is), istream_iterator<REAL>());
        p1.y = t.at(0);
        p2.y = t.at(1);
        is.clear();
        is.str( ini.request<string>("Rect_boundary", "z", "none") );
        t = vector<REAL>( istream_iterator<REAL>(is), istream_iterator<REAL>());
        p1.z = t.at(0);
        p2.z = t.at(1);
        int slice_num = ini.request<int>("Rect_boundary", "number_of_slices", -1);
        for (int i = 1; i <= slice_num; i++)
        {
            std::stringstream ss;
            string i_str;
            ss << i;
            ss >> i_str;
            slice_z.push_back(ini.request<REAL>("Rect_boundary", "slice" + i_str + "_z", 0));
            istringstream is( ini.request<string>("Rect_boundary", "slice" + i_str + "_normal", "none") );
            t = vector<REAL>( istream_iterator<REAL>(is), istream_iterator<REAL>());
            point p (t.at(0), t.at(1), t.at(2));
            p = p / p.norm();
            slice_normal.push_back(p);
        }
        std::string s = ini.request<string>("Rect_boundary", "is_continuous", "none");
        if ( s == "none" )
        {
            cout << "Error while reading ini file! (fracture_cross_array)";
        }
        is_continuous =  (s == "true" || s == "True" || s == "TRUE");
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

        if (is_continuous)
        {
            for (unsigned int i = 0; i < slice_z.size() + 1; i++)
            {
                std::vector<int> temp;
                temp.push_back(2 + i);
                temp.push_back(2 + 2 * (slice_z.size() + 1) + i);
                contacts.push_back(temp);
                temp = std::vector<int>();
                temp.push_back(2 + 1 * (slice_z.size() + 1) + i);
                temp.push_back(2 + 3 * (slice_z.size() + 1) + i);
                contacts.push_back(temp);
            }
        }

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
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p2.x, p1.y) + point(0, 0, -contact_shift)*i);
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p1.x, p1.y) + point(0, 0, -contact_shift)*i);
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p1.x, p2.y) + point(0, 0, -contact_shift)*i);
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p2.x, p2.y) + point(0, 0, -contact_shift)*i);
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p2.x, p1.y) + point(0, 0, -contact_shift)*(i+1));
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p1.x, p1.y) + point(0, 0, -contact_shift)*(i+1));
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p1.x, p2.y) + point(0, 0, -contact_shift)*(i+1));
            points.push_back(get_z_point(slice_normal.at(i), slice_z.at(i), p2.x, p2.y) + point(0, 0, -contact_shift)*(i+1));
        }
        points.push_back(point(p2.x, p1.y, p1.z) + point(0, 0, -contact_shift)*slice_z.size());
        points.push_back(point(p1.x, p1.y, p1.z) + point(0, 0, -contact_shift)*slice_z.size());
        points.push_back(point(p1.x, p2.y, p1.z) + point(0, 0, -contact_shift)*slice_z.size());
        points.push_back(point(p2.x, p2.y, p1.z) + point(0, 0, -contact_shift)*slice_z.size());

        set_facets();
    }

    void rect_boundary::set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount)
    {

        //contactFacesCount.push_back(contacts.size());
        int n = 0;
        for ( int i = 0; i < this->contacts.size(); i++ )
        {
            std::vector<int> c = this->contacts.at(i);
            contactFacesCount.push_back(facets.at(c.at(0)).trifacets.size());
            n += facets.at(c.at(0)).trifacets.size();
        }
        if (contacts.size() > n || contacts.size() == 0)
        {
                contactFacesCount.push_back(contacts.size() - n);
        }

        boundaryFacesCount.push_back(facets.at(0).trifacets.size());
        boundaryFacesCount.push_back(facets.at(1).trifacets.size());
        n = facets.at(0).trifacets.size() + facets.at(1).trifacets.size();
        if (0) // TODO
            for (int i = 0; i < 4; i++)
            {
                int_t m = 0;
                for ( int_t j = 0; j <= slice_z.size(); j++)
                    m += facets.at(2 + j + (slice_z.size()+1)*i).trifacets.size();
                boundaryFacesCount.push_back(m);
                n += m;
            }
        if (boundaries.size() > n)
            boundaryFacesCount.push_back(boundaries.size() - n);
    }

}
