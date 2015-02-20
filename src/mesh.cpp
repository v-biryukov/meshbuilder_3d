/*****************************************************************************
* name: mesh.h
*
* author: Biryukov V. biryukov.vova@gmail.com,  ...
*
* desc: Class for 3D mesh generation using tetgen1.5.0
*
* license: GPLv3
*
*****************************************************************************/


#include "profile/Profile.h"
#include <string.h>
#include <set>
#include <iterator>
#include "mesh.h"
#include "meshsplitter.h"

namespace swift
{

    using std::vector;
    using std::string;
    using std::stringstream;
    using std::copy;
    using std::sort;
    using std::cout;
    using std::endl;

    /*****************************************************************************
    *************  Mesh class  ***************************************************
    *****************************************************************************/
    /*****************************************************************************
    *  Constructor
    *****************************************************************************/
    mesh::mesh(char* path)
    {
        use_volume_constraints = false;
        read_from_file(path);
        init();
        set_points();
        create_facets();
        set_holes();
    }

    mesh::~mesh()
    {
        for (int i = 0; i < figures.size(); i++)
        {
            delete figures[i];
        }
    }

    /*****************************************************************************
    *  Private functions
    *****************************************************************************/

    REAL volume_constraint(REAL x, REAL y, REAL z)
    {
        //REAL t = (x*x + y*y + (z+20)*(z+20))/300;
        //return (1.1 - 1.0/(1.0+t*t));
        return 1;
    }

    int mesh::calculate_number_of_points()
    {
        int n = 0;
        for (vector<figure*>::size_type i = 0; i < figures.size(); i++)
            n += figures.at(i)->points.size();
        return n;
    }

    int mesh::calculate_number_of_trifacets()
    {
        int n = 0;
        for (vector<figure*>::size_type i = 0; i < figures.size(); i++)
            n += figures.at(i)->trifacets.size();
        return n;
    }

    int mesh::calculate_number_of_holes()
    {
        int n = 0;
        for (vector<figure*>::iterator it = figures.begin(); it != figures.end(); it++)
            if ((*it)->is_empty) n++;
        return n;
    }

    void mesh::read_from_file(string path)
    {
        using std::cin;
        Profile ini;
        // reading of the config-file
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
        quality   = ini.request<REAL>("Mesh", "quality", -1);

        average_step   = ini.request<REAL>("Mesh", "average_step", -1);
        segments.x = ini.request<int>("Segments", "number_of_segments_x", -1);
        segments.y = ini.request<int>("Segments", "number_of_segments_y", -1);
        segments.z = ini.request<int>("Segments", "number_of_segments_z", -1);
        int nof_figures = ini.request<int>("Figures", "number_of_figures", -1);
        for ( int i = 1; i <= nof_figures; i++ )
        {
            stringstream ss;
            string i_str;
            ss << i;
            ss >> i_str;
            string type = ini.request<string>("Figures", "figure" + i_str + "_type", "none");

            //if      (type == "Custom")               {figures.push_back(figure(				  path, average_step, 0));}

            if (type == "Cube")
            {
                cube * f = new cube( path, average_step, 0);
                figures.push_back(f);
            }
            else if (type == "FCA")
            {
                fracture_cross_array * f = new fracture_cross_array( path, average_step, 0);
                figures.push_back(f);
            }
            else if (type == "Fracture")
            {
                fracture * f = new fracture( path, average_step, 0);
                figures.push_back(f);
            }
            else if (type == "Rect_boundary")
            {
                rect_boundary * f = new rect_boundary( path, average_step, 0);
                figures.push_back(f);
            }
            else if (type == "Cross_fracture")
            {
                cross_fracture * f = new cross_fracture( path, average_step, 0);
                figures.push_back(f);
            }
            else if (type == "Ply_model")
            {
                ply_model * f = new ply_model( path, average_step, 0);
                figures.push_back(f);
            }
            else if (type == "Layered_boundary")
            {
                layered_boundary * f = new layered_boundary( path, average_step, 0);
                figures.push_back(f);
            }
            else
            {
                cout << "Error: there is no such type: " << type << ".";
                std::exit(1);
            }
            string s = ini.request<string>("Figures", "figure" + i_str + "_is_empty", "none");
            //std::cout << "em: " << s << std::endl;
            figures.back()->is_empty = (s == "true" || s == "True" || s == "TRUE");

            stringstream ss1(ini.request<string>("Figures", "figure" + i_str + "_position", "none"));
            ss1 >> figures.back()->pos.x >> figures.back()->pos.y >> figures.back()->pos.z;

            stringstream ss2(ini.request<string>("Figures", "figure" + i_str + "_angles", "none"));
            ss2 >> figures.back()->ang.alpha >> figures.back()->ang.beta >> figures.back()->ang.gamma;
            figures.back()->make_triangulation();

        }
    }

    void mesh::init()
    {
        in.numberofpoints = calculate_number_of_points();
        in.numberoffacets = calculate_number_of_trifacets();
        in.numberofholes = calculate_number_of_holes();
        // Creating arrays
        in.pointlist = new REAL[3*in.numberofpoints];
        in.facetlist = new tetgenio::facet[in.numberoffacets];
        in.facetmarkerlist = new int[in.numberoffacets];
        in.holelist = new REAL[3*in.numberofholes];
        //boundaries.resize(in.numberoffacets); ????????
    }

    void mesh::set_points()
    {
        int offset = 0;
        for (vector<figure*>::iterator it = figures.begin(); it != figures.end(); it++)
        {
            for (vector<point>::size_type i = 0; i < (*it)->points.size(); i++)
            {
                point t = (*it)->get_transformed_point(i);
                in.pointlist[3*(i + offset) + 0] = t.x;
                in.pointlist[3*(i + offset) + 1] = t.y;
                in.pointlist[3*(i + offset) + 2] = t.z;
            }
            offset += (*it)->points.size();
        }
    }

    void mesh::set_figure_boundaries(figure * f, int point_offset)
    {
        // Creating set of numbers of the non-contact facets
        vector<int> facet_nums = f->get_non_contact_facets();
        // Setting boundaries
        for ( unsigned int i = 0; i < facet_nums.size(); i++ )
        {
            int fn = facet_nums.at(i);
            for ( unsigned int j = 0; j < f->facets.at(fn).trifacets.size(); j++ )
            {
                int tfacn = f->facets.at(fn).trifacets.at(j);
                boundary_face tb = {int_t(f->trifacets.at(tfacn).points[0] + point_offset),
                                    int_t(f->trifacets.at(tfacn).points[1] + point_offset),
                                    int_t(f->trifacets.at(tfacn).points[2] + point_offset)};
                boundaries.push_back(tb);
            }
        }
        // Setting contacts
        for ( unsigned int i = 0; i < f->contacts.size(); i++ )
        {
            vector<int> c = f->contacts.at(i);
            for ( unsigned int j = 0; j < f->facets.at(c.at(0)).trifacets.size(); j++ )
            {
                int tfacn = f->facets.at(c.at(0)).trifacets.at(j);
                int dif = *(f->facets.at(c.at(1)).trifacets.begin()) - *(f->facets.at(c.at(0)).trifacets.begin());
                boundary_face c1 = {int_t(f->trifacets.at(tfacn).points[0] + point_offset),
                                    int_t(f->trifacets.at(tfacn).points[1] + point_offset),
                                    int_t(f->trifacets.at(tfacn).points[2] + point_offset)};
                boundary_face c2 = {int_t(f->trifacets.at(tfacn + dif).points[0] + point_offset),
                                    int_t(f->trifacets.at(tfacn + dif).points[1] + point_offset),
                                    int_t(f->trifacets.at(tfacn + dif).points[2] + point_offset)};
                contact_face temp;
                temp.faces[0] = c1;
                temp.faces[1] = c2;
                contacts.push_back(temp);
            }
        }
    }

    void mesh::set_boundaries()
    {
        int offset = 0;
        for ( unsigned int i = 0; i < figures.size(); i++ )
        {
            set_figure_boundaries(figures.at(i), offset);
            offset += figures.at(i)->points.size();
        }
    }

    void mesh::set_facet(int n_of_facet, boundary_face & b, int marker)
    {
        in.facetmarkerlist[n_of_facet] = marker;
        tetgenio::facet * f = &(in.facetlist[n_of_facet]);
        f->numberofpolygons = 1;
        f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
        f->numberofholes = 0;
        f->holelist = NULL;
        tetgenio::polygon * p = &f->polygonlist[0];
        p->numberofvertices = 3;
        p->vertexlist = new int[p->numberofvertices];
        p->vertexlist[0] = b.nodes[0];
        p->vertexlist[1] = b.nodes[1];
        p->vertexlist[2] = b.nodes[2];
    }

    void mesh::create_facets()
    {
        set_boundaries();
        for (unsigned int i = 0; i<boundaries.size(); i++)
            set_facet(i, boundaries.at(i));
        for (unsigned int i = 0; i<contacts.size(); i++)
        {
            set_facet(boundaries.size() + 2*i, contacts.at(i).faces[0], 2/*i + 1, 2*/);
            set_facet(boundaries.size() + 2*i + 1, contacts.at(i).faces[1], 2/*i + 1, 2*/);
        }
    }

    void mesh::set_holes()
    {
        int i = 0;
        for (vector<figure*>::iterator it = figures.begin(); it != figures.end(); it++)
            if ((*it)->is_empty)
            {
                point t = (*it)->transform((*it)->hole);
                in.holelist[3*i + 0] = t.x;
                in.holelist[3*i + 1] = t.y;
                in.holelist[3*i + 2] = t.z;
                i++;
                //std::cout << "h: " << t.x << " " << t.y << " " << t.z << std::endl;
            }
    }

    void mesh::set_volume_constraints(tetgenio * mid)
    {
        mid->tetrahedronvolumelist = new REAL[mid->numberoftetrahedra];
        for (int i = 0; i < mid->numberoftetrahedra; i++)
        {
            REAL t = average_step * volume_constraint(( mid->pointlist[3*mid->tetrahedronlist[4*i]+0] +
							mid->pointlist[3*mid->tetrahedronlist[4*i+1]+0] +
							mid->pointlist[3*mid->tetrahedronlist[4*i+2]+0] +
							mid->pointlist[3*mid->tetrahedronlist[4*i+3]+0])/4,
                                                      ( mid->pointlist[3*mid->tetrahedronlist[4*i]+1] +
							mid->pointlist[3*mid->tetrahedronlist[4*i+1]+1] +
							mid->pointlist[3*mid->tetrahedronlist[4*i+2]+1] +
							mid->pointlist[3*mid->tetrahedronlist[4*i+3]+1])/4,
                                                      ( mid->pointlist[3*mid->tetrahedronlist[4*i]+2] +
							mid->pointlist[3*mid->tetrahedronlist[4*i+1]+2] +
							mid->pointlist[3*mid->tetrahedronlist[4*i+2]+2] +
							mid->pointlist[3*mid->tetrahedronlist[4*i+3]+2])/4);
            mid->tetrahedronvolumelist[i] = t*t*t/6;
        }
    }

    /*****************************************************************************
    *  Public functions
    *****************************************************************************/

    void mesh::build()
    {
        stringstream conv_stream;
        // convert quality and average_step to char*
        conv_stream.clear();
        conv_stream.str("");
        conv_stream << quality;
        string str_quality;
        conv_stream >> str_quality;

        REAL a = average_step*average_step*average_step/6.0;
        conv_stream.clear();
        conv_stream.str("");
        conv_stream << std::fixed << a;
        string str_a;
        conv_stream >> str_a;

        string tetraparam;
        if (quality == 0.0)
            tetraparam = "pa" + str_a + "Y";
        else
            tetraparam = "pq" + str_quality + "a" + str_a + "Y";
        //string tetraparam = "qa10V";
        // Converting string to char* first
        char * tempparam = new char[tetraparam.size() + 1];
        copy(tetraparam.begin(), tetraparam.end(), tempparam);
        tempparam[tetraparam.size()] = '\0';
        in.save_nodes((char*)"in2");
        in.save_poly((char*)"in2");
        in.save_faces((char*)"in2");
        // Main calculations
        if (!use_volume_constraints)
        {
            cout << "Tetgen parameters = " << tempparam << endl;
            tetrahedralize(tempparam, &in, &out);
        }
        else
        {
            cout << "First tetgen parameters = " << tempparam << endl;
            tetgenio mid;
            tetrahedralize(tempparam, &in, &mid);
            if (quality == 0.0)
                tetraparam = "raa" + str_a + "Y";
            else
                tetraparam = "rq" + str_quality + "aa" + str_a + "Y";
            tempparam = new char[tetraparam.size() + 1];
            copy(tetraparam.begin(), tetraparam.end(), tempparam);
            tempparam[tetraparam.size()] = '\0';
            cout << "Second tetgen parameters = " << tempparam << endl;
            set_volume_constraints(&mid);
            tetrahedralize(tempparam, &mid, &out);
        }
        delete [] tempparam;
    }

    void mesh::save(char* filename)
    {
		in.save_nodes((char*)"in");
        in.save_poly((char*)"in");
        out.save_nodes(filename);
        out.save_elements(filename);
        out.save_faces(filename);
    }

    // It is needed only in split and save function
    // Isn't my code
    struct Vector3
        {
            REAL x, y, z;
            Vector3(){};
            Vector3(REAL xt, REAL yt, REAL zt)
            {
                x = xt;
                y = yt;
                z = zt;
            };
            Vector3 operator+(Vector3 & v)
            {
                return Vector3(x + v.x, y + v.y, z + v.z);
            };
            Vector3 operator*(REAL a)
            {
                return Vector3(a * x, a * y, a * z);
            };
        };
    struct CellAvgPoint
    {
        Vector3 point;
        int cellIndex;
        static bool CompareX(CellAvgPoint p1, CellAvgPoint p2)
        {
            return p1.point.x < p2.point.x;
        }
        static bool CompareY(CellAvgPoint p1, CellAvgPoint p2)
        {
            return p1.point.y < p2.point.y;
        }
        static bool CompareZ(CellAvgPoint p1, CellAvgPoint p2)
        {
            return p1.point.z < p2.point.z;
        }
    };

    void mesh::split_and_save()
    {
        // Split mesh on segments.x * segments.y * segments.z pieces and save
        // Isn't my code
        int_t cellsCount = out.numberoftetrahedra;
        int_t nodesCount = out.numberofpoints;
        cout << "cellsCount = " << cellsCount << "\n";
        typedef MeshSplitter::TransitionNode TransitionNode;
        Vector3 * vertices = new Vector3[nodesCount];
        for (int_t i = 0; i < nodesCount; i++)
        {
            vertices[i].x = out.pointlist[3*i + 0];
            vertices[i].y = out.pointlist[3*i + 1];
            vertices[i].z = out.pointlist[3*i + 2];
        }
        //int * cellIndices  = out.tetrahedronlist;
	    int_t * cellIndices  = new int_t [4 * cellsCount];
        int_t * meshIds      = new int_t [cellsCount];
        CellAvgPoint *cellPoints = new CellAvgPoint[cellsCount];
        for( int_t i = 0; i < cellsCount; i++)
        {
            meshIds[i] = 0;
            cellIndices[4*i+0] = int_t(out.tetrahedronlist[4*i+0]);
            cellIndices[4*i+1] = int_t(out.tetrahedronlist[4*i+1]);
            cellIndices[4*i+2] = int_t(out.tetrahedronlist[4*i+2]);
            cellIndices[4*i+3] = int_t(out.tetrahedronlist[4*i+3]);
            Vector3 avg = Vector3(0, 0, 0);
            avg = avg +  vertices[cellIndices[i * 4 + 0]];
            avg = avg +  vertices[cellIndices[i * 4 + 1]];
            avg = avg +  vertices[cellIndices[i * 4 + 2]];
            avg = avg +  vertices[cellIndices[i * 4 + 3]];
            avg = avg*0.25;
            cellPoints[i].point = avg;
            cellPoints[i].cellIndex = i;
        }
        int xSegmentsCount = segments.x;
        int ySegmentsCount = segments.y;
        int zSegmentsCount = segments.z;

        int currSegment;
        //typedef bool (*ftype)(CellAvgPoint,CellAvgPoint);
        sort(cellPoints, cellPoints + cellsCount, CellAvgPoint::CompareX);
        currSegment = 0;
        for(int_t i = 0; i < cellsCount; i++)
        {
            while((currSegment + 1 < xSegmentsCount) && (CellAvgPoint::CompareX(cellPoints[(currSegment + 1) * cellsCount / xSegmentsCount], cellPoints[i])))
                currSegment++;
            meshIds[cellPoints[i].cellIndex] += currSegment;
        }
        sort(cellPoints, cellPoints + cellsCount, CellAvgPoint::CompareY);
        currSegment = 0;
        for(int_t i = 0; i < cellsCount; i++)
        {
            while((currSegment + 1 < ySegmentsCount) && (CellAvgPoint::CompareY(cellPoints[(currSegment + 1) * cellsCount / ySegmentsCount], cellPoints[i])))
                currSegment++;
            meshIds[cellPoints[i].cellIndex] += xSegmentsCount * currSegment;
        }
        sort(cellPoints, cellPoints + cellsCount, CellAvgPoint::CompareZ);
        currSegment = 0;
        for(int_t i = 0; i < cellsCount; i++)
        {
            while((currSegment + 1 < zSegmentsCount) && (CellAvgPoint::CompareZ(cellPoints[(currSegment + 1) * cellsCount / zSegmentsCount], cellPoints[i])))
                currSegment++;
            meshIds[cellPoints[i].cellIndex] += xSegmentsCount * ySegmentsCount * currSegment;
        }
        delete [] cellPoints;

        // Complications here ////////////////////////////////////////////

        cout << "Splitting mesh" << endl;
        int_t subMeshesCount = 1;
        int_t * subMeshNodesCount = new int_t[subMeshesCount];
        subMeshNodesCount[0] = out.numberofpoints;

        vector<int_t> contactFacesCount;
        vector<int_t> boundaryFacesCount;


        figures[0]->set_boundaries_and_contacts(boundaries, contacts, boundaryFacesCount, contactFacesCount);


        // ///////////////////////////////////////////////////////////////

        MeshSplitter mesh_splitter;
        mesh_splitter.LoadBaseMeshes(cellIndices, meshIds, cellsCount, subMeshNodesCount, subMeshesCount,
                                        contacts.data(), contactFacesCount.data(), contactFacesCount.size(),
                                        boundaries.data(), boundaryFacesCount.data(), boundaryFacesCount.size());
        cout << "Mesh was split successfully" << endl << endl;

        int_t meshesCount = mesh_splitter.GetMeshesCount();
        int_t maxNodesCount = 0;
        int_t maxCellsCount = 0;
        for(int_t meshIndex = 0; meshIndex < meshesCount; meshIndex++)
        {
            if(maxNodesCount < mesh_splitter.GetNodesCount(meshIndex)) maxNodesCount = mesh_splitter.GetNodesCount(meshIndex);
            if(maxCellsCount < mesh_splitter.GetCellsCount(meshIndex)) maxCellsCount = mesh_splitter.GetCellsCount(meshIndex);
        }
        int_t *localCellIndicesBuf          = new int_t       [maxCellsCount * 4];
        int_t *localNodeGlobalIndicesBuf    = new int_t       [maxNodesCount];
        Vector3   *localVerticesBuf       = new Vector3         [maxNodesCount];
        int_t *localSubmeshNodesCount       = new int_t       [subMeshesCount];

        cout << "Submesh count = " << subMeshesCount << "\n";
        cout << "Separate mesh files now will be saved" << endl << endl;
        cout << "Global mesh info:" << endl << "  nodes: " << nodesCount << endl << "  cells: " << cellsCount << endl << endl << endl;

        for(int_t meshIndex = 0; meshIndex < meshesCount; meshIndex++)
        {

            string filePath = "Data/";
            stringstream fileName; fileName << "Mesh" << meshIndex << ".sm";
            string fileFullName = filePath + fileName.str();
            cout << "Saving mesh " << fileFullName << endl;
            std::ofstream outFile;
            outFile.open(fileFullName.c_str(), std::ios::out | std::ios::binary);
            int_t localCellsCount = mesh_splitter.GetCellsCount(meshIndex);
            int_t localNodesCount = mesh_splitter.GetNodesCount(meshIndex);
            outFile.write((const char*)&localCellsCount, sizeof(int_t));
            outFile.write((const char*)&localNodesCount, sizeof(int_t));
            cout << fileName.str() << " info:" << endl << "  nodes: " << localNodesCount << endl << "  cells: " << localCellsCount << endl;
            mesh_splitter.GetCellLocalIndices(meshIndex, localCellIndicesBuf);
            outFile.write((const char*)localCellIndicesBuf, localCellsCount * 4 * sizeof(int_t));
            mesh_splitter.GetLocalNodesGlobalIndices(meshIndex, localNodeGlobalIndicesBuf);
            for(int_t localNodeIndex = 0; localNodeIndex < localNodesCount; localNodeIndex++)
            {
                localVerticesBuf[localNodeIndex] = vertices[localNodeGlobalIndicesBuf[localNodeIndex]];
            }
            outFile.write((const char*)localVerticesBuf, localNodesCount * sizeof(Vector3));
            int_t submeshesCount = mesh_splitter.GetLocalSubmeshesCount(meshIndex);
            mesh_splitter.GetLocalSubmeshNodesCount(meshIndex, localSubmeshNodesCount);
            outFile.write((const char*)&submeshesCount, sizeof(int_t));
            outFile.write((const char*)localSubmeshNodesCount, sizeof(int_t) * submeshesCount);
            int_t localContactFacesCount = 0;
            int_t contactTypesCount = mesh_splitter.GetLocalContactTypesCount(meshIndex);
            outFile.write((const char*)&contactTypesCount, sizeof(int_t));
            for(int_t contactTypeIndex = 0; contactTypeIndex < contactTypesCount; contactTypeIndex++)
            {
                int_t facesCount = mesh_splitter.GetLocalContactFacesCount(meshIndex, contactTypeIndex);
                outFile.write((const char*)&facesCount, sizeof(int_t));
                localContactFacesCount += facesCount;
            }
            //outFile.write((const char*)&localContactFacesCount, sizeof(int_t));
            if(localContactFacesCount > 0)
            {
                contact_face *contactFaces  = new contact_face[localContactFacesCount];
                mesh_splitter.GetLocalContactFaces(meshIndex, contactFaces);
                outFile.write((const char*)contactFaces, sizeof(contact_face) * localContactFacesCount);
                delete [] contactFaces;
            }
            int_t localBoundaryFacesCount = 0;
            int_t boundaryTypesCount = mesh_splitter.GetLocalBoundaryTypesCount(meshIndex);
            outFile.write((const char*)&boundaryTypesCount, sizeof(int_t));
            for(int_t boundaryTypeIndex = 0; boundaryTypeIndex < boundaryTypesCount; boundaryTypeIndex++)
            {
                int_t facesCount = mesh_splitter.GetLocalBoundaryFacesCount(meshIndex, boundaryTypeIndex);
                outFile.write((const char*)&facesCount, sizeof(int_t));
                localBoundaryFacesCount += facesCount;
            }
            //outFile.write((const char*)&localBoundaryFacesCount, sizeof(int_t));
            if(localBoundaryFacesCount > 0)
            {
                boundary_face *boundaryFaces  = new boundary_face[localBoundaryFacesCount];
                mesh_splitter.GetLocalBoundaryFaces(meshIndex, boundaryFaces);
                outFile.write((const char*)boundaryFaces, sizeof(boundary_face) * localBoundaryFacesCount);
                delete [] boundaryFaces;
            }
            cout << "writing shared mesh info" << endl;
            int_t sharedDomainsCount = mesh_splitter.GetSharedRegionsCount(meshIndex);
            outFile.write((const char*)&sharedDomainsCount, sizeof(int_t));
            for(int_t regionIndex = 0; regionIndex < sharedDomainsCount; regionIndex++)
            {
                int_t dstMeshIndex = mesh_splitter.GetSharedRegionDstRegionId(meshIndex, regionIndex);
                outFile.write((const char*)&dstMeshIndex, sizeof(int_t));
                int_t sharedCellsCount = mesh_splitter.GetSharedCellsCount(meshIndex, regionIndex);
                outFile.write((const char*)&sharedCellsCount, sizeof(int_t));
                int_t *sharedIndicesBuf = new int_t[sharedCellsCount * 4];
                mesh_splitter.GetSharedCells(meshIndex, regionIndex, sharedIndicesBuf);
                outFile.write((const char*)sharedIndicesBuf, sharedCellsCount * 4 * sizeof(int_t));
                int_t transitionNodesCount = mesh_splitter.GetTransitionNodesCount(meshIndex, regionIndex);;
                outFile.write((const char*)&transitionNodesCount, sizeof(int_t));
                TransitionNode *transitionNodesBuf = new TransitionNode[transitionNodesCount];
                mesh_splitter.GetTransitionNodes(meshIndex, regionIndex, transitionNodesBuf);
                outFile.write((const char*)transitionNodesBuf, transitionNodesCount * sizeof(TransitionNode));
                delete sharedIndicesBuf;
                delete transitionNodesBuf;
            }
            outFile.close();
            cout << endl;
            //Debug
            cout << "Saving .node file" << endl;
            fileName.clear();
            fileName.str("");
            fileName << "Mesh" << meshIndex << ".node";
            fileFullName = filePath + fileName.str();
            outFile.open(fileFullName.c_str(), std::ios::out);
            outFile << localNodesCount;
            outFile << " 3 0 0\n";
            for(int_t localNodeIndex = 0; localNodeIndex < localNodesCount; localNodeIndex++)
            {
                outFile << localNodeIndex << " ";
                outFile << localVerticesBuf[localNodeIndex].x << " ";
                outFile << localVerticesBuf[localNodeIndex].y << " ";
                outFile << localVerticesBuf[localNodeIndex].z << "\n";
            }
            outFile.close();
            cout << "Saving .ele file" << endl;
            fileName.clear();
            fileName.str("");
            fileName << "Mesh" << meshIndex << ".ele";
            fileFullName = filePath + fileName.str();
            outFile.open(fileFullName.c_str(), std::ios::out);
            outFile << localCellsCount;
            outFile << " 4 0\n";
            for( int_t localCellIndex = 0; localCellIndex < localCellsCount; localCellIndex++)
            {
                outFile << localCellIndex << " ";
                outFile << localCellIndicesBuf[localCellIndex * 4 + 0] << " ";
                outFile << localCellIndicesBuf[localCellIndex * 4 + 1] << " ";
                outFile << localCellIndicesBuf[localCellIndex * 4 + 2] << " ";
                outFile << localCellIndicesBuf[localCellIndex * 4 + 3] << "\n";
            }
            outFile.close();
        }
        /**/
    }
}
/*****************************************************************************
*
*****************************************************************************/



//nullptr

void process(swift::mesh m)
{
    m.build();
    m.save((char*)"out");
    m.split_and_save();
}

int main(int argc, char ** argv)
{
    if (argc >= 2)
    {
        process(swift::mesh (argv[1]));
    }
    else
    {
        process(swift::mesh ((char*)"meshbuilder.ini"));
    }
}
