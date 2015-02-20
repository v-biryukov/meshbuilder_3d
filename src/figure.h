/*****************************************************************************
* name: figure.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Class for representing an arbitrary figure in a mesh.
*
* license: GPLv3
*
*****************************************************************************/

#pragma once
#include <vector>
#include <sstream>
#include <fstream>
#include "point.h"
#include "edge.h"
#include "facet.h"
#include "settings.h"

namespace swift
{
    struct boundary_face
    {
      int_t nodes[3];
    };

    struct contact_face
    {
      boundary_face faces[2];
    };

    struct figure
    {
        std::vector<point> points;
        std::vector<edge> edges;
        std::vector<facet> facets;
        std::vector<std::vector<int> > contacts;

        std::vector<trifacet> trifacets;

        bool is_empty;
        point hole;
        REAL av_step;
        REAL (*constraints)(REAL, REAL, REAL);

        point pos;
        struct {REAL alpha, beta, gamma;} ang;

        figure(){};
        figure(std::string path, REAL av_step_t, REAL (*constraints_t)(REAL, REAL, REAL) = 0);
        figure(std::vector<point> & tpoints, std::vector<edge> & tedges, std::vector<facet> & tfacets, REAL av_step_t, point hole_t);
        void make_triangulation();
        virtual void read_from_file(std::string path);
        virtual void set_data() = 0;
        virtual void set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount) = 0;
        void set_edges_by_facets();
        point get_transformed_point(int i);
        point transform(point p);
        std::vector<int> get_non_contact_facets();
    };


    figure::figure(std::string path, REAL av_step_t, REAL (*constraints_t)(REAL, REAL, REAL))
    {
        //av_step = av_step_t;
        //constraints = constraints_t;
        //read_from_file(path);
        //set_data();
    }

    figure::figure(std::vector<point> & tpoints, std::vector<edge> & tedges, std::vector<facet> & tfacets, REAL av_step_t, point hole_t = point(0, 0, 0))
    {
        av_step = av_step_t;
        points = tpoints;
        edges = tedges;
        facets = tfacets;
    }

    //void figure::set_data(){};
    //void figure::set_boundaries_and_contacts(const std::vector<boundary_face> & boundaries, const std::vector<contact_face> & contacts, std::vector<unsigned int> & boundaryFacesCount, std::vector<unsigned int> & contactFacesCount){};


    void figure::make_triangulation()
    {
        set_edges_by_facets();
        std::vector<int> v = get_non_contact_facets();
        for ( unsigned int i = 0; i < contacts.size(); i++ )
        {
            std::cout << "Triangulating contact facet " << i+1 << " from " << contacts.size() <<std::endl;
            std::vector<int> c = contacts.at(i);
            facets.at(c.at(0)).make_triangulation(points, trifacets, edges, av_step);
            //facets.at(c.at(1)).make_triangulation(points, trifacets, edges, av_step);
            facets.at(c.at(1)).take_triangulation(points, trifacets, edges, facets.at(c.at(0)));
        }
        for ( unsigned int i = 0; i < v.size(); i++ )
        {
            std::cout << "Triangulating facet " << i+1 << " from " << v.size() <<std::endl;
            facets.at(v.at(i)).make_triangulation(points, trifacets, edges, av_step);
        }
    }

    void figure::set_edges_by_facets()
    {
        for ( unsigned int i = 0; i < facets.size(); i++ )
        {
            std::cout << "Setting facet " << i+1 << " from " << facets.size() <<std::endl;
            facet f = facets.at(i);
            for (std::vector<int>::size_type i = 0; i < f.points.size(); i++)
            {
                edge e = edge(f.points.at(i), f.points.at((i+1) % f.points.size()));
                if (std::find(edges.begin(), edges.end(), e) == edges.end())
                {
                    edges.push_back(e);
                    f.edges.push_back(edges.size()-1);
                }
            }
        }
    }

    point figure::get_transformed_point(int i)
    {
        return pos + points[i].rotate(ang.alpha, ang.beta, ang.gamma);
    }
    point figure::transform(point p)
    {
        return pos + p.rotate(ang.alpha, ang.beta, ang.gamma);
    }

    std::vector<int> figure::get_non_contact_facets()
    {
        std::vector<int> contact_facets;
        std::vector<int> result;
        for (std::vector<int>::size_type i = 0; i < contacts.size(); i++)
            contact_facets.insert(contact_facets.end(), contacts.at(i).begin(), contacts.at(i).end());
        for (std::vector<int>::size_type i = 0; i < facets.size(); i++)
            if(std::find(contact_facets.begin(), contact_facets.end(), i) == contact_facets.end())
                result.push_back(i);
        return result;
    }

    void figure::read_from_file(std::string path)
    {
        std::ifstream file;
        std::stringstream ss;
        file.open(path.c_str());
        std::string s;
        //file >> s;
        //std::getline(file, s);
        // Reading hole info
        std::getline(file, s);
        std::istringstream is( s );
        std::vector<REAL> hole_v = std::vector<REAL>( std::istream_iterator<REAL>(is),std::istream_iterator<REAL>());
        hole.x = hole_v.at(0);
        hole.y = hole_v.at(1);
        hole.z = hole_v.at(2);
        // Reading points info
        int num_of_points;
        file >> num_of_points;
        std::getline(file, s);
        for (int i = 0; i < num_of_points; i++)
        {
            ss.clear();
            std::getline(file, s);
            ss << s;
            point p;
            ss >> p.x;
            ss >> p.y;
            ss >> p.z;
            points.push_back(p);
        }
        // Reading facets info
        int num_of_facets;
        file >> num_of_facets;
        std::getline(file, s);
        std::string line;
        for (int i = 0; i < num_of_facets; i++)
        {
            std::getline(file, line);
            std::istringstream is( line );
            std::vector<int> foo = std::vector<int>( std::istream_iterator<int>(is), std::istream_iterator<int>() );
            facet f(foo);
            facets.push_back(f);
        }
        int num_of_contacts;
        file >> num_of_contacts;
        std::getline(file, line);
        for (int i = 0; i < num_of_contacts; i++)
        {
            std::getline(file, line);
            std::istringstream is( line );
            contacts.push_back(std::vector<int>( std::istream_iterator<int>(is), std::istream_iterator<int>()));
        }
    }
}
