/*****************************************************************************
* name: mesh.h
*
* author: Biryukov V. biryukov.vova@gmail.com
*
* desc: Class for 3D mesh generation using tetgen1.5.0
*
* license: GPLv3
*
*****************************************************************************/


#pragma once
#include "tetgen.h"
#include <iostream>
#include <vector>
#include "figure.h"
#include "figures/fracture.h"
#include "figures/fracture_cross_array.h"
#include "figures/cross_fracture.h"
#include "figures/cube.h"
#include "figures/rect_boundary.h"
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

    class mesh
    {
    private:
        //figure main_boundary;
        std::vector<figure> figures;
        tetgenio in, out;
        struct {int x, y, z;} segments;
        REAL quality, average_step;
        std::vector<boundary_face> boundaries;
        std::vector<contact_face> contacts;

        void init();
        void read_from_file(std::string path);
        void set_points();
        void set_figure_boundaries(figure & f, int point_offset);
        void set_boundaries();
        void set_facet(int n_of_facet, boundary_face & b, int marker = 0);
        void create_facets();
        void set_holes();
        int calculate_number_of_points();
        int calculate_number_of_trifacets();
        int calculate_number_of_holes();
        //REAL (*volume_constraint)(REAL x, REAL y, REAL z);
        bool use_volume_constraints;
        void set_volume_constraints(tetgenio * mid);
        //REAL volume_constraint(REAL x, REAL y, REAL z);
    public:
        mesh(){};
        mesh(char* path);
        void build();
        void save(char* filename);
        void split_and_save();
    };

}
