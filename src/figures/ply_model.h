/*****************************************************************************
* name: ply_model.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Derived class from figure. Represents a custom ply model.
*
* license: GPLv3
*
*****************************************************************************/

#pragma once
#include "../figure.h"
#include "../profile/Profile.h"

namespace swift
{
    struct ply_model : public figure
    {
        std::string path_to_model;
        double scale;

        ply_model(std::string path, double av_step_t, REAL (*constraints_t)(REAL, REAL, REAL) = 0)
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

    void ply_model::read_from_file(std::string path)
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
        scale = ini.request<REAL>("Ply_model", "scale", -1);
        path_to_model = ini.request<std::string>("Ply_model", "path_to_model", "none");
        if ( path_to_model == "none" )
        {
            cout << "Error while reading ini file! (ply_model)";
            std::exit(1);
        }

    }

    void ply_model::set_data()
    {
        using std::string;
        using std::cout;
        using std::cin;
        using std::endl;

        unsigned int number_of_points, number_of_faces;
        // Считываем файл
        std::ifstream infile(path_to_model.c_str());
        std::string line;
        std::getline(infile, line);
        std::stringstream ss(line);
        assert(ss.str() == "ply");

        // find number of points
        while (line.substr(0, 15) != "element vertex ")
            std::getline(infile, line);
        ss.str("");
        ss.clear();
        ss << line.substr(15);
        ss >> number_of_points;

        // find number of faces
        while (line.substr(0, 13) != "element face ")
            std::getline(infile, line);
        ss.str("");
        ss.clear();
        ss << line.substr(12);
        ss >> number_of_faces;

        while (line.substr(0, 10) != "end_header")
            std::getline(infile, line);

        point p;
        std::vector<int> fs;

        for (unsigned int i = 0; i < number_of_points; i++)
        {
            std::getline(infile, line);
            std::stringstream ss2(line);
            if (!(ss2 >> p.x >> p.y >> p.z)) { break; }
            points.push_back(scale * p);
        }
        int n_of_points_in_face;
        for (unsigned int i = 0; i < number_of_faces; i++)
        {
            //fs.resize(0);
            std::getline(infile, line);
            std::istringstream ss2(line);
            if (!(ss2 >> n_of_points_in_face)) { break; }
            fs.resize(n_of_points_in_face);
            for (int j = 0; j < n_of_points_in_face; j++)
                ss2 >> fs[j];

            facets.push_back(facet(fs));
        }
    }
    void ply_model::set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount)
    {

    }

}
