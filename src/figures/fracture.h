/*****************************************************************************
* name: fracture.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Derived class from figure. Represents a geological fracture.
*
* license: GPLv3
*
*****************************************************************************/

#pragma once
#include "../figure.h"
#include "../profile/Profile.h"

namespace swift
{
    struct fracture : public figure
    {
        bool is_contact;
        REAL height, length;
        REAL thickness;
        REAL hpart, lpart;

        fracture(std::string path, double av_step_t, REAL (*constraints_t)(REAL, REAL, REAL) = 0)
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


    void fracture::read_from_file(std::string path)
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
            cout << "Error while reading ini file! (fracture) Press any key to close." << endl;
            cin.get();
            std::exit(1);
        }
        height = ini.request<REAL>("Fracture", "height", -1);
        length = ini.request<REAL>("Fracture", "length", -1);
        thickness = ini.request<REAL>("Fracture", "thickness", -1);
        hpart = ini.request<REAL>("Fracture", "hpart", -1);
        lpart = ini.request<REAL>("Fracture", "lpart", -1);

        std::string s = ini.request<std::string>("Fracture", "is_contact", "none");
        if ( s == "none" )
        {
            cout << "Error while reading ini file! (fracture)";
        }
        is_contact =  (s == "true" || s == "True" || s == "TRUE");
    }

    void fracture::set_data()
    {
        points.push_back(point(0, -length/2, -height/2));
        points.push_back(point(0, -length/2,  height/2));
        points.push_back(point(0,  length/2,  height/2));
        points.push_back(point(0,  length/2, -height/2));
        points.push_back(point(-thickness/2, -lpart*length/2, -hpart*height/2));
        points.push_back(point(-thickness/2, -lpart*length/2,  hpart*height/2));
        points.push_back(point(-thickness/2,  lpart*length/2,  hpart*height/2));
        points.push_back(point(-thickness/2,  lpart*length/2, -hpart*height/2));
        points.push_back(point(thickness/2, -lpart*length/2, -hpart*height/2));
        points.push_back(point(thickness/2, -lpart*length/2,  hpart*height/2));
        points.push_back(point(thickness/2,  lpart*length/2,  hpart*height/2));
        points.push_back(point(thickness/2,  lpart*length/2, -hpart*height/2));

        facets.push_back(facet(0, 1, 5, 4));
        facets.push_back(facet(0, 1, 9, 8));
        facets.push_back(facet(1, 2, 6, 5));
        facets.push_back(facet(1, 2, 10, 9));
        facets.push_back(facet(2, 3, 7, 6));
        facets.push_back(facet(2, 3, 11, 10));
        facets.push_back(facet(3, 0, 4, 7));
        facets.push_back(facet(3, 0, 8, 11));
        facets.push_back(facet(4, 5, 6, 7));
        facets.push_back(facet(8, 9, 10, 11));

        if (is_contact)
        {
            // Strange errors here. Can`t use {} initialisation;
            std::vector<int> temp(2);
            temp[0] = 0; temp[1] = 1;
            contacts.push_back(temp);
            temp[0] = 2; temp[1] = 3;
            contacts.push_back(temp);
            temp[0] = 4; temp[1] = 5;
            contacts.push_back(temp);
            temp[0] = 6; temp[1] = 7;
            contacts.push_back(temp);
            temp[0] = 8; temp[1] = 9;
            contacts.push_back(temp);
        }
    }

    void fracture::set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount)
    {

    }

}
