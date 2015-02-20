/*****************************************************************************
* name: cross_fracture.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Derived class from figure. Represents 2 geological intersecting fractures.
*
* license: GPLv3
*
*****************************************************************************/

#pragma once
#include "../figure.h"
#include "profile/Profile.h"
#include <cmath>

namespace swift
{
    struct cross_fracture : public figure
    {
        bool is_contact;
        REAL height, length;
        REAL thickness;
        REAL hpart, lpart;
        REAL angle;

        cross_fracture(std::string path, double av_step_t,REAL (*constraints_t)(REAL, REAL, REAL) = 0);
        void read_from_file(std::string path);
        void set_data();
        void set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount);
    };

    cross_fracture::cross_fracture(std::string path, double av_step_t, REAL (*constraints_t)(REAL, REAL, REAL))
    {
        constraints = constraints_t;
        av_step = av_step_t;
        read_from_file(path);
        set_data();
    }

    void cross_fracture::read_from_file(std::string path)
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
        height = ini.request<REAL>("Cross_fracture", "height", -1);
        length = ini.request<REAL>("Cross_fracture", "length", -1);
        thickness = ini.request<REAL>("Cross_fracture", "thickness", -1);
        hpart = ini.request<REAL>("Cross_fracture", "hpart", -1);
        lpart = ini.request<REAL>("Cross_fracture", "lpart", -1);
        angle = ini.request<REAL>("Cross_fracture", "angle", -1);
        std::string s = ini.request<std::string>("Cross_fracture", "is_contact", "none");
        if ( s == "none" )
        {
            cout << "Error while reading ini file! (Cross_fracture)";
        }
        is_contact =  (s == "true" || s == "True" || s == "TRUE");
    }

    void cross_fracture::set_data()
    {
        point angp1 = point(+length/2*sin(angle/2), length/2*cos(angle/2), 0.0);
        point angp2 = point(-length/2*sin(angle/2), length/2*cos(angle/2), 0.0);
        point hangp1 = angp1 * lpart;
        point hangp2 = angp2 * lpart;
        point zp = point(0, 0, height/2);
        point hzp = zp * hpart;
        point th1 = point(thickness/2*cos(angle/2), -thickness/2*sin(angle/2), 0.0);
        point th2 = point(thickness/2*cos(angle/2), +thickness/2*sin(angle/2), 0.0);
        point crossp = point(0.0, thickness/(2*sin(angle/2)), 0.0);

        points.push_back(-angp1 - zp);
        points.push_back(-angp2 - zp);
        points.push_back(-zp);
        points.push_back(angp2 - zp);
        points.push_back(angp1 - zp);


        points.push_back(-hangp1 + th1 - hzp);
        points.push_back(-hangp2 - th2 - hzp);
        points.push_back(-hangp1 - th1 - hzp);
        points.push_back(-hangp2 + th2 - hzp);

        points.push_back(-crossp - hzp);

        points.push_back(-crossp - 2*th1 - hzp);
        points.push_back(-crossp + 2*th2 - hzp);
        points.push_back(+crossp - 2*th2 - hzp);
        points.push_back(+crossp + 2*th1 - hzp);

        points.push_back(+crossp - hzp);

        points.push_back(hangp2 - th2 - hzp);
        points.push_back(hangp1 + th1 - hzp);
        points.push_back(hangp2 + th2 - hzp);
        points.push_back(hangp1 - th1 - hzp);



        points.push_back(-hangp1 + th1 + hzp);
        points.push_back(-hangp2 - th2 + hzp);
        points.push_back(-hangp1 - th1 + hzp);
        points.push_back(-hangp2 + th2 + hzp);

        points.push_back(-crossp + hzp);

        points.push_back(-crossp - 2*th1 + hzp);
        points.push_back(-crossp + 2*th2 + hzp);
        points.push_back(+crossp - 2*th2 + hzp);
        points.push_back(+crossp + 2*th1 + hzp);

        points.push_back(+crossp + hzp);

        points.push_back(hangp2 - th2 + hzp);
        points.push_back(hangp1 + th1 + hzp);
        points.push_back(hangp2 + th2 + hzp);
        points.push_back(hangp1 - th1 + hzp);


        points.push_back(-angp1 + zp);
        points.push_back(-angp2 + zp);
        points.push_back(+zp);
        points.push_back(angp2 + zp);
        points.push_back(angp1 + zp);

        facets.push_back(facet(0, 7, 21, 33));
        facets.push_back(facet(0, 5, 19, 33));
        facets.push_back(facet(1, 6, 20, 34));
        facets.push_back(facet(1, 8, 22, 34));

        facets.push_back(facet(0, 7, 10, 2));
        facets.push_back(facet(0, 5, 9, 2));
        facets.push_back(facet(1, 8, 11, 2));
        facets.push_back(facet(1, 6, 9, 2));

        facets.push_back(facet(2, 10, 12));
        facets.push_back(facet(2, 11, 13));

        facets.push_back(facet(2, 3, 15, 12));
        facets.push_back(facet(2, 3, 17, 14));
        facets.push_back(facet(2, 4, 16, 13));
        facets.push_back(facet(2, 4, 18, 14));

        facets.push_back(facet(3, 17, 31, 36));
        facets.push_back(facet(3, 15, 29, 36));
        facets.push_back(facet(4, 18, 32, 37));
        facets.push_back(facet(4, 16, 30, 37));

        facets.push_back(facet(7, 10, 24, 21));
        facets.push_back(facet(5, 9, 23, 19));
        facets.push_back(facet(8, 11, 25, 22));
        facets.push_back(facet(6, 9, 23, 20));

        facets.push_back(facet(10, 12, 26, 24));
        facets.push_back(facet(11, 13, 27, 25));

        facets.push_back(facet(12, 15, 29, 26));
        facets.push_back(facet(14, 17, 31, 28));
        facets.push_back(facet(13, 16, 30, 27));
        facets.push_back(facet(14, 18, 32, 28));

        facets.push_back(facet(21, 24, 35, 33));
        facets.push_back(facet(19, 23, 35, 33));
        facets.push_back(facet(20, 23, 35, 34));
        facets.push_back(facet(22, 25, 35, 34));

        facets.push_back(facet(24, 26, 35));
        facets.push_back(facet(25, 27, 35));

        facets.push_back(facet(29, 26, 35, 36));
        facets.push_back(facet(31, 28, 35, 36));
        facets.push_back(facet(32, 28, 35, 37));
        facets.push_back(facet(30, 27, 35, 37));

        if (is_contact)
        {
            std::vector<int> temp(2);
            for (int i = 0; i < facets.size()/2; i++)
            {
                temp[0] = 2*i;
                temp[1] = 2*i + 1;
                contacts.push_back(temp);
            }
        }
        hole.x = 0.0;
        hole.y = 0.0;
        hole.z = 0.0;

    }

    void cross_fracture::set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount)
    {

    }

}
