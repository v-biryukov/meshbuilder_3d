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
#include "figures/ply_model.h"
#include "figures/layered_boundary.h"
#include "settings.h"

namespace swift
{



    class mesh
    {
    private:
        //std::vector<figure> figures;
        std::vector<figure*> figures;
        tetgenio in, out;
        struct {int x, y, z;} segments;
        REAL quality, average_step;
        std::vector<boundary_face> boundaries;
        std::vector<contact_face> contacts;

        void init();
        void read_from_file(std::string path);
        void set_points();
        void set_figure_boundaries(figure * f, int point_offset);
        void set_boundaries();
        void set_facet(int n_of_facet, boundary_face & b, int marker = 0);
        void create_facets();
        void set_holes();
        int calculate_number_of_points();
        int calculate_number_of_trifacets();
        int calculate_number_of_holes();
        bool use_volume_constraints;
        void set_volume_constraints(tetgenio * mid);
    public:
        mesh(){};
        mesh(char* path);
        ~mesh();
        void build();
        void save(char* filename);
        void split_and_save();
    };
}
