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
#include "../profile/Profile.h"

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
        virtual void set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount);

    };


    void cube::read_from_file(std::string path)
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
        size = ini.request<REAL>("Cube", "size", -1);
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


    void cube::set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount)
    {
        int n = 0;
        for (int i = 0; i < this->facets.size(); i++)
            n += this->facets[i].trifacets.size();

        boundaryFacesCount.push_back(n);
    }




}
