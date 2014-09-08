/*****************************************************************************
* name: meshsplitter.h
*
* license: GPLv3
*
*****************************************************************************/

#include <vector>
#include <algorithm>
namespace swift
{
typedef int_t IndexType;
typedef contact_face ContactFace;
typedef boundary_face BoundaryFace;


template<typename T>
class DuplicateRemover
{
public:
  void AddElement(const T& elem)
  {
    elements.push_back(elem);
  }
  IndexType RemoveDuplicates()
  {
    std::sort(elements.begin(), elements.end());

    IndexType size = IndexType(elements.size());

    if(size == 0) return 0;

    IndexType resultSize = 1;

    for(IndexType i = 1; i < size; i++)
    {
      if(!(elements[resultSize - 1] == elements[i]))
      {
        elements[resultSize++] = elements[i];
      }
    }
    elements.resize(resultSize);
    return resultSize;
  }

  T &operator [](IndexType num)
  {
    return elements[num];
  }

  T *GetElements()
  {
    return &(elements[0]);
  }
  IndexType GetElementsCount()
  {
    return IndexType(elements.size());
  }
private:
  std::vector<T> elements;
};

  class RegionBuilder
  {
  public:
    RegionBuilder(){}
    ~RegionBuilder()
    {
      delete [] incidentRegionsPool;
      delete [] nodeInfo;
    }
    void LoadMesh(IndexType *cellIndices, IndexType *cellRegionId, IndexType cellsCount,
                  ContactFace *contactFaces, IndexType contactFacesCount)
    {
      nodesCount = 0;
      for(IndexType cellIndex = 0; cellIndex < cellsCount; cellIndex++)
      {
        for(IndexType i = 0; i < 4; i++)
        {
          if(cellIndices[cellIndex * 4 + i] > nodesCount) nodesCount = cellIndices[cellIndex * 4 + i];
        }
      }
      nodesCount++; //max index + 1

      nodeInfo = new NodeInfo[nodesCount];

      for(IndexType nodeIndex = 0; nodeIndex < nodesCount; nodeIndex++)
      {
        nodeInfo[nodeIndex].incidentRegionsCount = 0;
      }

      IndexType totalIncidentRegionsCount = 0;

      for(IndexType cellIndex = 0; cellIndex < cellsCount; cellIndex++)
      {
        //IndexType regionId = cellRegionId[cellIndex];
        for(IndexType i = 0; i < 4; i++)
        {
          IndexType nodeIndex = cellIndices[4 * cellIndex + i];
          nodeInfo[nodeIndex].incidentRegionsCount++;
          totalIncidentRegionsCount++;
        }
      }
      for( unsigned int i = 0; i < contactFacesCount; i++)
      {
        for(int j = 0; j < 3; j++)
        {
          int nodeIndices[2];
          nodeIndices[0] = contactFaces[i].faces[0].nodes[j];
          nodeIndices[1] = contactFaces[i].faces[1].nodes[j];

          int count0 = nodeInfo[nodeIndices[0]].incidentRegionsCount;
          int count1 = nodeInfo[nodeIndices[1]].incidentRegionsCount;

          nodeInfo[nodeIndices[0]].incidentRegionsCount += count1;
          totalIncidentRegionsCount += count1;

          nodeInfo[nodeIndices[1]].incidentRegionsCount += count0;
          totalIncidentRegionsCount += count0;
        }
      }
      incidentRegionsPool = new IndexType[totalIncidentRegionsCount];
      IndexType poolOffset = 0;
      for(IndexType nodeIndex = 0; nodeIndex < nodesCount; nodeIndex++)
      {
        nodeInfo[nodeIndex].incidentRegions = incidentRegionsPool + poolOffset;
        poolOffset += nodeInfo[nodeIndex].incidentRegionsCount;
        nodeInfo[nodeIndex].incidentRegionsCount = 0; //will be recomputed later
      }

      for(IndexType cellIndex = 0; cellIndex < cellsCount; cellIndex++)
      {
        IndexType regionId = cellRegionId[cellIndex];
        for(IndexType i = 0; i < 4; i++)
        {
          IndexType nodeIndex = cellIndices[4 * cellIndex + i];
          bool found = 0;
          for(IndexType regions = 0; ((regions < nodeInfo[nodeIndex].incidentRegionsCount) && !found); regions++)
          {
            if(nodeInfo[nodeIndex].incidentRegions[regions] == regionId) found = 1;
          }
          if(!found) nodeInfo[nodeIndex].incidentRegions[nodeInfo[nodeIndex].incidentRegionsCount++] = regionId;
        }
      }



      for(IndexType contactFaceIndex = 0; contactFaceIndex < contactFacesCount; contactFaceIndex++)
      {
        for(IndexType faceNode = 0; faceNode < 3; faceNode++)
        {
          IndexType nodeIndices[2];
          nodeIndices[0] = contactFaces[contactFaceIndex].faces[0].nodes[faceNode];
          nodeIndices[1] = contactFaces[contactFaceIndex].faces[1].nodes[faceNode];


          for(IndexType srcRegionIndex = 0; srcRegionIndex < nodeInfo[nodeIndices[0]].incidentRegionsCount; srcRegionIndex++)
          {
            IndexType srcRegion = nodeInfo[nodeIndices[0]].incidentRegions[srcRegionIndex];
            bool found = 0;
            for(IndexType dstRegionIndex = 0; ((dstRegionIndex < nodeInfo[nodeIndices[1]].incidentRegionsCount) && !found); dstRegionIndex++)
            {
              if(nodeInfo[nodeIndices[1]].incidentRegions[dstRegionIndex] == srcRegion) found = 1;
            }
            if(!found) nodeInfo[nodeIndices[1]].incidentRegions[nodeInfo[nodeIndices[1]].incidentRegionsCount++] = srcRegion;
          }

          for(IndexType srcRegionIndex = 0; srcRegionIndex < nodeInfo[nodeIndices[1]].incidentRegionsCount; srcRegionIndex++)
          {
            IndexType srcRegion = nodeInfo[nodeIndices[1]].incidentRegions[srcRegionIndex];
            bool found = 0;
            for(IndexType dstRegionIndex = 0; ((dstRegionIndex < nodeInfo[nodeIndices[0]].incidentRegionsCount) && !found); dstRegionIndex++)
            {
              if(nodeInfo[nodeIndices[0]].incidentRegions[dstRegionIndex] == srcRegion) found = 1;
            }
            if(!found) nodeInfo[nodeIndices[0]].incidentRegions[nodeInfo[nodeIndices[0]].incidentRegionsCount++] = srcRegion;
          }
        }
      }
    }

    void LoadMeshByNodes(IndexType *cellIndices, IndexType *nodeRegionId, IndexType cellsCount,
                  ContactFace *contactFaces, IndexType contactFacesCount)
    {
      nodesCount = 0;
      for(IndexType cellIndex = 0; cellIndex < cellsCount; cellIndex++)
      {
        for(IndexType i = 0; i < 4; i++)
        {
          if(cellIndices[cellIndex * 4 + i] > nodesCount) nodesCount = cellIndices[cellIndex * 4 + i];
        }
      }
      nodesCount++; //max index + 1

      nodeInfo = new NodeInfo[nodesCount];

      for(IndexType nodeIndex = 0; nodeIndex < nodesCount; nodeIndex++)
      {
        nodeInfo[nodeIndex].incidentRegionsCount = 0;
      }

      IndexType totalIncidentRegionsCount = 0;

      for(IndexType cellIndex = 0; cellIndex < cellsCount; cellIndex++)
      {
        IndexType cellRegionsCount = 1;
        IndexType localRegions[4];
        for(IndexType i = 0; i < 4; i++)
        {
          localRegions[i] = nodeRegionId[cellIndices[4 * cellIndex + i]];
        }
        std::sort(localRegions, localRegions + 4);
        for(IndexType i = 0; i < 3; i++)
        {
          if(localRegions[i] != localRegions[i + 1]) cellRegionsCount++;
        }

        for(IndexType i = 0; i < 4; i++)
        {
          nodeInfo[cellIndices[4 * cellIndex + i]].incidentRegionsCount = cellRegionsCount;
          totalIncidentRegionsCount += cellRegionsCount;
        }
      }

      for(IndexType contactFaceIndex = 0; contactFaceIndex < contactFacesCount; contactFaceIndex++)
      {
        for(IndexType faceNode = 0; faceNode < 3; faceNode++)
        {
          IndexType nodeIndices[2];
          nodeIndices[0] = contactFaces[contactFaceIndex].faces[0].nodes[faceNode];
          nodeIndices[1] = contactFaces[contactFaceIndex].faces[1].nodes[faceNode];

          IndexType count0 = nodeInfo[nodeIndices[0]].incidentRegionsCount;
          IndexType count1 = nodeInfo[nodeIndices[1]].incidentRegionsCount;

          nodeInfo[nodeIndices[0]].incidentRegionsCount += count1;
          totalIncidentRegionsCount += count1;

          nodeInfo[nodeIndices[1]].incidentRegionsCount += count0;
          totalIncidentRegionsCount += count0;
        }
      }

      incidentRegionsPool = new IndexType[totalIncidentRegionsCount];
      IndexType poolOffset = 0;
      for(IndexType nodeIndex = 0; nodeIndex < nodesCount; nodeIndex++)
      {
        nodeInfo[nodeIndex].incidentRegions = incidentRegionsPool + poolOffset;
        poolOffset += nodeInfo[nodeIndex].incidentRegionsCount;
        nodeInfo[nodeIndex].incidentRegionsCount = 0; //will be recomputed later
      }

      for(IndexType cellIndex = 0; cellIndex < cellsCount; cellIndex++)
      {
        IndexType regionIds[4];
        for(IndexType i = 0; i < 4; i++)
        {
          regionIds[i] = nodeRegionId[cellIndices[4 * cellIndex + i]];
        }
        for(IndexType regionNumber = 0; regionNumber < 4; regionNumber++)
        {
          for(IndexType i = 0; i < 4; i++)
          {
            IndexType nodeIndex = cellIndices[4 * cellIndex + i];
            bool found = 0;
            for(IndexType regions = 0; ((regions < nodeInfo[nodeIndex].incidentRegionsCount) && !found); regions++)
            {
              if(nodeInfo[nodeIndex].incidentRegions[regions] == regionIds[regionNumber]) found = 1;
            }
            if(!found) nodeInfo[nodeIndex].incidentRegions[nodeInfo[nodeIndex].incidentRegionsCount++] = regionIds[regionNumber];
          }
        }
      }



      for(IndexType contactFaceIndex = 0; contactFaceIndex < contactFacesCount; contactFaceIndex++)
      {
        for(IndexType faceNode = 0; faceNode < 3; faceNode++)
        {
          IndexType nodeIndices[2];
          nodeIndices[0] = contactFaces[contactFaceIndex].faces[0].nodes[faceNode];
          nodeIndices[1] = contactFaces[contactFaceIndex].faces[1].nodes[faceNode];


          for(IndexType srcRegionIndex = 0; srcRegionIndex < nodeInfo[nodeIndices[0]].incidentRegionsCount; srcRegionIndex++)
          {
            IndexType srcRegion = nodeInfo[nodeIndices[0]].incidentRegions[srcRegionIndex];
            bool found = 0;
            for(IndexType dstRegionIndex = 0; ((dstRegionIndex < nodeInfo[nodeIndices[1]].incidentRegionsCount) && !found); dstRegionIndex++)
            {
              if(nodeInfo[nodeIndices[1]].incidentRegions[dstRegionIndex] == srcRegion) found = 1;
            }
            if(!found) nodeInfo[nodeIndices[1]].incidentRegions[nodeInfo[nodeIndices[1]].incidentRegionsCount++] = srcRegion;
          }

          for(IndexType srcRegionIndex = 0; srcRegionIndex < nodeInfo[nodeIndices[1]].incidentRegionsCount; srcRegionIndex++)
          {
            IndexType srcRegion = nodeInfo[nodeIndices[1]].incidentRegions[srcRegionIndex];
            bool found = 0;
            for(IndexType dstRegionIndex = 0; ((dstRegionIndex < nodeInfo[nodeIndices[0]].incidentRegionsCount) && !found); dstRegionIndex++)
            {
              if(nodeInfo[nodeIndices[0]].incidentRegions[dstRegionIndex] == srcRegion) found = 1;
            }
            if(!found) nodeInfo[nodeIndices[0]].incidentRegions[nodeInfo[nodeIndices[0]].incidentRegionsCount++] = srcRegion;
          }
        }
      }
    }
    IndexType GetNodesCount()
    {
      return nodesCount;
    }
    void GetNodeRegions(IndexType nodeIndex, IndexType *incidentRegions)
    {
      if(nodeIndex >= nodesCount) return;
      for( unsigned int i = 0; i < nodeInfo[nodeIndex].incidentRegionsCount; i++)
      {
        incidentRegions[i] = nodeInfo[nodeIndex].incidentRegions[i];
      }
    }
    IndexType GetNodeRegionsCount(IndexType nodeIndex)
    {
      if(nodeIndex >= nodesCount) return 0;
      return nodeInfo[nodeIndex].incidentRegionsCount;
    }
  private:
    struct NodeInfo
    {
      IndexType incidentRegionsCount;
      IndexType *incidentRegions;
    };
    NodeInfo *nodeInfo;
    IndexType nodesCount;

    IndexType *incidentRegionsPool;
  };

  class MeshSplitter
  {
  public:
    MeshSplitter()
    {
    }
    void LoadBaseMeshes(IndexType *cellIndices, IndexType *cellRegionId, IndexType cellsCount,
                        IndexType *subMeshNodesCount, IndexType subMeshesCount,
                        ContactFace *contactFaces, IndexType *contactFacesCount, IndexType contactTypesCount,
                        BoundaryFace *boundaryFaces, IndexType *boundaryFacesCount, IndexType boundaryTypesCount)
    {
      IndexType totalContactFacesCount = 0;
      IndexType totalBoundaryFacesCount = 0;

      for(IndexType contactTypeIndex = 0; contactTypeIndex < contactTypesCount; contactTypeIndex++)
      {
        totalContactFacesCount += contactFacesCount[contactTypeIndex];
      }
      for(IndexType boundaryTypeIndex = 0; boundaryTypeIndex < boundaryTypesCount; boundaryTypeIndex++)
      {
        totalBoundaryFacesCount += boundaryFacesCount[boundaryTypeIndex];
      }
      meshesCount = 0;
      for(IndexType cellIndex = 0; cellIndex < cellsCount; cellIndex++)
      {
        if(cellRegionId[cellIndex] > meshesCount) meshesCount = cellRegionId[cellIndex];
      }
      meshesCount++; //meshes count is one more than max meshid
      localMeshes = new LocalMesh[meshesCount];

      for(IndexType meshIndex = 0; meshIndex < meshesCount; meshIndex++)
      {
        localMeshes[meshIndex].nodesCount = 0;
        localMeshes[meshIndex].cellsCount = 0;
      }

      /*IndexType lastIndex = 0;
      this->subMeshes = new SubMeshInfo[subMeshesCount];
      this->subMeshesCount = subMeshesCount;
      for(IndexType subMeshIndex = 0; subMeshIndex < subMeshesCount; subMeshIndex++)
      {
        //this->subMeshes[subMeshIndex].subMeshId = subMeshIds[subMeshIndex];
        this->subMeshes[subMeshIndex].firstNodeIndex = lastIndex;
        this->subMeshes[subMeshIndex].nodesCount = subMeshNodesCount[subMeshIndex];
        lastIndex += subMeshNodesCount[subMeshIndex];
      }*/
      ComputeExpandedIndices(cellIndices, cellRegionId, cellsCount, contactFaces, totalContactFacesCount);
      ComputeLocalMeshCells();
      ComputeNodeInfo();
      ComputeTransitionNodes();
      ComputeSharedCellTransitionIndices();
      ComputeLocalNodeIndices();
      ComputeLocalSubmeshes(subMeshNodesCount, subMeshesCount);
      ComputeLocalContactFaces(contactFaces, contactFacesCount, contactTypesCount);
      ComputeLocalBoundaryFaces(boundaryFaces, boundaryFacesCount, boundaryTypesCount);
    }
    ~MeshSplitter()
    {
      delete [] nodeGlobalIndicesPool;
      delete [] incidentRegionIdPool;
      delete [] localIndexPool;
      delete [] nodeInfo;
      delete [] cellGlobalIndicesPool;
      delete [] localMeshes;
    }

    IndexType GetMeshesCount()
    {
      return meshesCount;
    }

    IndexType GetNodesCount(IndexType regionId)
    {
      return localMeshes[regionId].nodesCount;
    }
    IndexType GetCellsCount(IndexType regionId)
    {
      return localMeshes[regionId].cellsCount;
    }

    void GetCellLocalIndices(IndexType regionId, IndexType *cellLocalIndices)
    {
      for(IndexType cellIndex = 0; cellIndex < localMeshes[regionId].cellsCount; cellIndex++)
      {
        for(IndexType i = 0; i < 4; i++)
        {
          IndexType nodeGlobalIndex = localMeshes[regionId].cellGlobalIndices[cellIndex * 4 + i];
          cellLocalIndices[cellIndex * 4 + i] = GetNodeLocalIndex(regionId, nodeGlobalIndex);
        }
      }
    }

    IndexType GetLocalContactTypesCount(IndexType regionId)
    {
      return localMeshes[regionId].contactTypesCount;
    }
    IndexType GetLocalContactFacesCount(IndexType regionId, IndexType contactType)
    {
      return localMeshes[regionId].contactFacesCount[contactType];
    }

    void GetLocalContactFaces(IndexType regionId, ContactFace *contactFaces)
    {
      IndexType totalContactFacesCount = 0;
      for(IndexType contactTypeIndex = 0; contactTypeIndex < localMeshes[regionId].contactTypesCount; contactTypeIndex++)
      {
        totalContactFacesCount += localMeshes[regionId].contactFacesCount[contactTypeIndex];
      }

      for(IndexType contactFaceIndex = 0; contactFaceIndex < totalContactFacesCount; contactFaceIndex++)
      {
        contactFaces[contactFaceIndex] = localMeshes[regionId].contactFaces[contactFaceIndex];
      }
    }

    IndexType GetLocalBoundaryTypesCount(IndexType regionId)
    {
      return localMeshes[regionId].boundaryTypesCount;
    }
    IndexType GetLocalBoundaryFacesCount(IndexType regionId, IndexType boundaryType)
    {
      return localMeshes[regionId].boundaryFacesCount[boundaryType];
    }

    void GetLocalBoundaryFaces(IndexType regionId, BoundaryFace *boundaryFaces)
    {
      IndexType totalBoundaryFacesCount = 0;
      for(IndexType boundaryTypeIndex = 0; boundaryTypeIndex < localMeshes[regionId].boundaryTypesCount; boundaryTypeIndex++)
      {
        totalBoundaryFacesCount += localMeshes[regionId].boundaryFacesCount[boundaryTypeIndex];
      }

      for(IndexType boundaryFaceIndex = 0; boundaryFaceIndex < totalBoundaryFacesCount; boundaryFaceIndex++)
      {
        boundaryFaces[boundaryFaceIndex] = localMeshes[regionId].boundaryFaces[boundaryFaceIndex];
      }
    }

    IndexType GetLocalSubmeshesCount(IndexType regionId)
    {
      return localMeshes[regionId].subMeshesCount;
    }

    void GetLocalSubmeshNodesCount(IndexType regionId, IndexType *nodesCount)
    {
      for(IndexType submeshIndex = 0; submeshIndex < localMeshes[regionId].subMeshesCount; submeshIndex++)
      {
        nodesCount[submeshIndex] = localMeshes[regionId].subMeshNodesCount[submeshIndex];
      }
    }

    void GetLocalNodesGlobalIndices(IndexType regionId, IndexType *globalIndices)
    {
      for(IndexType localNodeIndex = 0; localNodeIndex < localMeshes[regionId].nodesCount; localNodeIndex++)
      {
        globalIndices[localNodeIndex] = localMeshes[regionId].nodeGlobalIndices[localNodeIndex];
      }
    }

    IndexType GetSharedRegionsCount   (IndexType regionId)
    {
      return IndexType(localMeshes[regionId].destRegionId.size());
    }

    IndexType GetSharedRegionDstRegionId(IndexType regionId, IndexType regionIndex)
    {
      return localMeshes[regionId].destRegionId[regionIndex];
    }

    IndexType GetSharedCellsCount     (IndexType regionId, IndexType regionIndex)
    {
      return localMeshes[regionId].sharedCellsCount[regionIndex];
    }

    IndexType GetTransitionNodesCount (IndexType regionId, IndexType regionIndex)
    {
      return localMeshes[regionId].transitionNodesCount[regionIndex];
    }

    struct TransitionNode
    {
      IndexType nativeIndex;
      IndexType targetIndex;
    };
    void      GetTransitionNodes      (IndexType regionId, IndexType regionIndex, TransitionNode *transitionNode)
    {
      for(IndexType transitionNodeIndex = 0; transitionNodeIndex < localMeshes[regionId].transitionNodesCount[regionIndex]; transitionNodeIndex++)
      {
        IndexType nodeGlobalIndex = localMeshes[regionId].transitionNodesGlobalIndices[regionIndex][transitionNodeIndex];
        transitionNode[transitionNodeIndex].nativeIndex = GetNodeLocalIndex(regionId, nodeGlobalIndex);
        transitionNode[transitionNodeIndex].targetIndex = GetNodeLocalIndex(localMeshes[regionId].destRegionId[regionIndex], nodeGlobalIndex);
      }
    }
    void      GetSharedCells          (IndexType regionId, IndexType regionIndex, IndexType *transitionIndices)
    {
      for(IndexType cellIndex = 0; cellIndex < localMeshes[regionId].sharedCellsCount[regionIndex]; cellIndex++)
      {
        for(IndexType i = 0; i < 4; i++)
        {
          //transitionIndices[4 * cellIndex + i] = localMeshes[regionId].sharedCellsGlobalIndices[regionIndex][cellIndex * 4 + i];
          transitionIndices[4 * cellIndex + i] = localMeshes[regionId].sharedCellsTransitionIndices[regionIndex][cellIndex * 4 + i];
        }
      }
    }

  private:

    IndexType GetNodeLocalIndex(IndexType regionId, IndexType nodeGlobalIndex)
    {
      for(IndexType incidentRegion = 0; incidentRegion < nodeInfo[nodeGlobalIndex].incidentRegionsCount; incidentRegion++)
      {
        if(nodeInfo[nodeGlobalIndex].incindentRegionId[incidentRegion] == regionId)
        {
          return nodeInfo[nodeGlobalIndex].localIndex[incidentRegion];
        }
      }
      return -1;
    }


    void ComputeExpandedIndices(
      IndexType *cellIndices, IndexType *cellRegionId, IndexType cellsCount,
      ContactFace *contactFaces, IndexType contactFacesCount)
    {
      RegionBuilder *regionBuilder = new RegionBuilder();
      regionBuilder->LoadMesh(cellIndices, cellRegionId, cellsCount, contactFaces, contactFacesCount);
      IndexType *cellRegions = new IndexType[meshesCount];
      IndexType *nodeRegions = new IndexType[meshesCount];

      IndexType sharedCellsTotalCount = 0;
      //IndexType sharedRegionsTotalCount = 0;
      for(IndexType cellIndex = 0; cellIndex < cellsCount; cellIndex++)
      {
        IndexType cellRegionsCount = 0;
        for(IndexType i = 0; i < 4; i++)
        {
          IndexType nodeIndex = cellIndices[cellIndex * 4 + i];

          IndexType nodeRegionsCount = regionBuilder->GetNodeRegionsCount(nodeIndex);
          regionBuilder->GetNodeRegions(nodeIndex, nodeRegions);

          for(IndexType nodeRegionIndex = 0; nodeRegionIndex < nodeRegionsCount; nodeRegionIndex++)
          {
            bool found = 0;
            for(IndexType cellRegionIndex = 0; ((cellRegionIndex < cellRegionsCount) && (!found)); cellRegionIndex++)
            {
              if(nodeRegions[nodeRegionIndex] == cellRegions[cellRegionIndex])
                found = 1;
            }
            if(!found) cellRegions[cellRegionsCount++] = nodeRegions[nodeRegionIndex];
          }
        }

        IndexType sourceRegion = cellRegionId[cellIndex];;//-1;

        if(sourceRegion != -1)
        {
          for(IndexType regionIndex = 0; regionIndex < cellRegionsCount; regionIndex++)
          {
            if(sourceRegion != cellRegions[regionIndex])
            {
              bool found = 0;
              IndexType dstRegion = 0;
              for(IndexType dstRegionIndex = 0; dstRegionIndex < IndexType(localMeshes[sourceRegion].destRegionId.size()); dstRegionIndex++)
              {
                if(localMeshes[sourceRegion].destRegionId[dstRegionIndex] == cellRegions[regionIndex])
                {
                  dstRegion = dstRegionIndex;
                  found = 1;
                  break;
                }
              }
              if(!found)
              {
                dstRegion = IndexType(localMeshes[sourceRegion].destRegionId.size());
                localMeshes[sourceRegion].destRegionId.push_back(cellRegions[regionIndex]);
                localMeshes[sourceRegion].transitionNodesCount.push_back(0);
                localMeshes[sourceRegion].sharedCellsCount.push_back(0);
              }

              localMeshes[sourceRegion].sharedCellsCount[dstRegion]++;
              sharedCellsTotalCount++;
            }
          }
        }else
        {
          //something's wrong
        }
      }

      sharedCellsGlobalIndicesPool = new IndexType[sharedCellsTotalCount * 4];
      sharedCellsTransitionIndicesPool = new IndexType[sharedCellsTotalCount * 4];

      IndexType offset = 0;

      for(IndexType meshIndex = 0; meshIndex < meshesCount; meshIndex++)
      {
        localMeshes[meshIndex].sharedCellsGlobalIndices = new IndexType*[localMeshes[meshIndex].destRegionId.size()];
        localMeshes[meshIndex].sharedCellsTransitionIndices = new IndexType*[localMeshes[meshIndex].destRegionId.size()];

        for(IndexType dstRegion = 0; dstRegion < IndexType(localMeshes[meshIndex].destRegionId.size()); dstRegion++)
        {
          localMeshes[meshIndex].sharedCellsGlobalIndices[dstRegion] = sharedCellsGlobalIndicesPool + offset * 4;
          localMeshes[meshIndex].sharedCellsTransitionIndices[dstRegion] = sharedCellsTransitionIndicesPool + offset * 4;

          offset += localMeshes[meshIndex].sharedCellsCount[dstRegion];
          localMeshes[meshIndex].sharedCellsCount[dstRegion] = 0; //will be recomputed
        }
      }

      normalCellsCount = cellsCount;
      expandedCellsCount = cellsCount + sharedCellsTotalCount;

      expandedCellIndices = new IndexType[expandedCellsCount * 4];
      expandedCellRegionId = new IndexType[expandedCellsCount];

      expandedCellsCount = cellsCount; //will be expanded further


      for(IndexType cellIndex = 0; cellIndex < cellsCount; cellIndex++)
      {
        for(IndexType i = 0; i < 4; i++)
        {
          expandedCellIndices[cellIndex * 4 + i] = cellIndices[cellIndex * 4 + i];
        }
        expandedCellRegionId[cellIndex] = cellRegionId[cellIndex];
      }

      for(IndexType cellIndex = 0; cellIndex < cellsCount; cellIndex++)
      {
        //debug
        {
          int matches = 0;
          for(IndexType i = 0; i < 4; i++)
          {
            if(cellIndices[cellIndex * 4 + i] == 3950) matches++;
            if(cellIndices[cellIndex * 4 + i] == 5631) matches++;
            if(cellIndices[cellIndex * 4 + i] == 5632) matches++;
          }
          if (matches == 3)
          {
            //int pp = 1;
          }
        }//debug
        IndexType cellRegionsCount = 0;
        for(IndexType i = 0; i < 4; i++)
        {
          IndexType nodeIndex = cellIndices[cellIndex * 4 + i];

          IndexType nodeRegionsCount = regionBuilder->GetNodeRegionsCount(nodeIndex);
          regionBuilder->GetNodeRegions(nodeIndex, nodeRegions);

          for(IndexType nodeRegionIndex = 0; nodeRegionIndex < nodeRegionsCount; nodeRegionIndex++)
          {
            bool found = 0;
            for(IndexType cellRegionIndex = 0; ((cellRegionIndex < cellRegionsCount) && (!found)); cellRegionIndex++)
            {
              if(nodeRegions[nodeRegionIndex] == cellRegions[cellRegionIndex])
                found = 1;
            }
            if(!found) cellRegions[cellRegionsCount++] = nodeRegions[nodeRegionIndex];
          }
        }

        IndexType sourceRegion = cellRegionId[cellIndex];//-1;
        if(sourceRegion != -1)
        {
          for(IndexType regionIndex = 0; regionIndex < cellRegionsCount; regionIndex++)
          {
            if(sourceRegion != cellRegions[regionIndex])
            {
              bool found = 0;
              IndexType dstRegion = 0;
              for(IndexType dstRegionIndex = 0; dstRegionIndex < IndexType(localMeshes[sourceRegion].destRegionId.size()); dstRegionIndex++)
              {
                if(localMeshes[sourceRegion].destRegionId[dstRegionIndex] == cellRegions[regionIndex])
                {
                  dstRegion = dstRegionIndex;
                  found = 1;
                  break;
                }
              }
              if(!found)
              {
                assert(0);
              }


              for(IndexType i = 0; i < 4; i++)
              {
                expandedCellIndices[expandedCellsCount * 4 + i] = cellIndices[cellIndex * 4 + i];

                localMeshes[sourceRegion].sharedCellsGlobalIndices[dstRegion][localMeshes[sourceRegion].sharedCellsCount[dstRegion] * 4 + i] =
                  cellIndices[cellIndex * 4 + i];
              }

              expandedCellRegionId[expandedCellsCount] = cellRegions[regionIndex];
              expandedCellsCount++;

              localMeshes[sourceRegion].sharedCellsCount[dstRegion]++;
            }
          }
        }else
        {
          assert(0);
        }
      }

      delete [] nodeRegions;
      delete [] cellRegions;
      delete regionBuilder;
    }


    void ComputeLocalMeshCells()
    {
      IndexType cellGlobalIndicesPoolSize = 0;
      for(IndexType cellIndex = 0; cellIndex < expandedCellsCount; cellIndex++)
      {
        localMeshes[expandedCellRegionId[cellIndex]].cellsCount++;
        cellGlobalIndicesPoolSize++;
      }
      cellGlobalIndicesPool = new IndexType[cellGlobalIndicesPoolSize * 4];

      IndexType offset = 0;
      for(IndexType meshIndex = 0; meshIndex < meshesCount; meshIndex++)
      {
        localMeshes[meshIndex].cellGlobalIndices = cellGlobalIndicesPool + offset * 4;
        offset += localMeshes[meshIndex].cellsCount;
        localMeshes[meshIndex].cellsCount = 0; //will be restored
      }

      for(IndexType cellIndex = 0; cellIndex < expandedCellsCount; cellIndex++)
      {
        for(IndexType i = 0; i < 4; i++)
        {
          localMeshes[expandedCellRegionId[cellIndex]].cellGlobalIndices[localMeshes[expandedCellRegionId[cellIndex]].cellsCount * 4 + i] = expandedCellIndices[cellIndex * 4 + i];
        }
        localMeshes[expandedCellRegionId[cellIndex]].cellsCount++;
      }
    }



    void ComputeNodeInfo()
    {
      RegionBuilder *regionBuilder = new RegionBuilder();
      regionBuilder->LoadMesh(expandedCellIndices, expandedCellRegionId, expandedCellsCount, 0, 0);

      nodesCount = regionBuilder->GetNodesCount();
      nodeInfo = new NodeInfo[nodesCount];

      IndexType nodeInfoPoolSize = 0;
      for(IndexType nodeIndex = 0; nodeIndex < nodesCount; nodeIndex++)
      {
        nodeInfo[nodeIndex].incidentRegionsCount = regionBuilder->GetNodeRegionsCount(nodeIndex);
        nodeInfoPoolSize += nodeInfo[nodeIndex].incidentRegionsCount;
      }

      incidentRegionIdPool = new IndexType[nodeInfoPoolSize];
      localIndexPool = new IndexType[nodeInfoPoolSize];

      IndexType offset = 0;

      for(IndexType nodeIndex = 0; nodeIndex < nodesCount; nodeIndex++)
      {
        nodeInfo[nodeIndex].incindentRegionId = incidentRegionIdPool + offset;
        nodeInfo[nodeIndex].localIndex = localIndexPool + offset;
        offset += nodeInfo[nodeIndex].incidentRegionsCount;

        regionBuilder->GetNodeRegions(nodeIndex, nodeInfo[nodeIndex].incindentRegionId);
      }

      IndexType *maxMeshNodeIndex = new IndexType[meshesCount];
      for(IndexType meshIndex = 0; meshIndex < meshesCount; meshIndex++)
      {
        maxMeshNodeIndex[meshIndex] = 0;
      }

      for(IndexType nodeIndex = 0; nodeIndex < nodesCount; nodeIndex++)
      {
        for(IndexType incidentRegion = 0; incidentRegion < nodeInfo[nodeIndex].incidentRegionsCount; incidentRegion++)
        {
          IndexType incidentRegionIndex = nodeInfo[nodeIndex].incindentRegionId[incidentRegion];
          nodeInfo[nodeIndex].localIndex[incidentRegion] = maxMeshNodeIndex[incidentRegionIndex];
          maxMeshNodeIndex[incidentRegionIndex]++;
        }
      }
      delete maxMeshNodeIndex;

      /*for(IndexType nodeIndex = 0; nodeIndex < nodesCount; nodeIndex++)
      {
        for(IndexType incidentRegion = 0; incidentRegion < nodeInfo[nodeIndex].incidentRegionsCount; incidentRegion++)
        {
          IndexType incidentRegionIndex = nodeInfo[nodeIndex].incindentRegionId[incidentRegion];
          localMesh[incidentRegionIndex].nodesCount++;
        }
      }*/

      delete regionBuilder;
    }

    void ComputeTransitionNodes()
    {
      IndexType transitionNodesPoolSize = 0;
      for(IndexType srcMeshIndex = 0; srcMeshIndex < meshesCount; srcMeshIndex++)
      {
        for(IndexType dstMesh = 0; dstMesh < IndexType(localMeshes[srcMeshIndex].destRegionId.size()); dstMesh++)
        {
          //IndexType dstMeshIndex = localMeshes[srcMeshIndex].destRegionId[dstMesh];
          DuplicateRemover<IndexType> nodeFinder;

          for(IndexType cellIndex = 0; cellIndex < localMeshes[srcMeshIndex].sharedCellsCount[dstMesh]; cellIndex++)
          {
            for(int i = 0; i < 4; i++)
              nodeFinder.AddElement(localMeshes[srcMeshIndex].sharedCellsGlobalIndices[dstMesh][4 * cellIndex + i]);
          }
          nodeFinder.RemoveDuplicates();

          localMeshes[srcMeshIndex].transitionNodesCount[dstMesh] = nodeFinder.GetElementsCount();
          transitionNodesPoolSize += localMeshes[srcMeshIndex].transitionNodesCount[dstMesh];
        }
      }

      transitionNodesGlobalIndicesPool = new IndexType[transitionNodesPoolSize];

      IndexType offset = 0;
      for(IndexType srcMeshIndex = 0; srcMeshIndex < meshesCount; srcMeshIndex++)
      {
        localMeshes[srcMeshIndex].transitionNodesGlobalIndices = new IndexType*[localMeshes[srcMeshIndex].destRegionId.size()];
        for(IndexType dstMesh = 0; dstMesh < IndexType(localMeshes[srcMeshIndex].destRegionId.size()); dstMesh++)
        {
          localMeshes[srcMeshIndex].transitionNodesGlobalIndices[dstMesh] = transitionNodesGlobalIndicesPool + offset;
          offset += localMeshes[srcMeshIndex].transitionNodesCount[dstMesh];
        }
      }


      for(IndexType srcMeshIndex = 0; srcMeshIndex < meshesCount; srcMeshIndex++)
      {
        for(IndexType dstMesh = 0; dstMesh < IndexType(localMeshes[srcMeshIndex].destRegionId.size()); dstMesh++)
        {
          //IndexType dstMeshIndex = localMeshes[srcMeshIndex].destRegionId[dstMesh];
          DuplicateRemover<IndexType> nodeFinder;

          for(IndexType cellIndex = 0; cellIndex < localMeshes[srcMeshIndex].sharedCellsCount[dstMesh]; cellIndex++)
          {
            for(int i = 0; i < 4; i++)
              nodeFinder.AddElement(localMeshes[srcMeshIndex].sharedCellsGlobalIndices[dstMesh][4 * cellIndex + i]);
          }
          nodeFinder.RemoveDuplicates();

          for(IndexType transitionNode = 0; transitionNode < nodeFinder.GetElementsCount(); transitionNode++)
          {
            localMeshes[srcMeshIndex].transitionNodesGlobalIndices[dstMesh][transitionNode] = nodeFinder[transitionNode];
          }
        }
      }
    }

    void ComputeSharedCellTransitionIndices()
    {
      for(IndexType srcMeshIndex = 0; srcMeshIndex < meshesCount; srcMeshIndex++)
      {
        for(IndexType dstMesh = 0; dstMesh < IndexType(localMeshes[srcMeshIndex].destRegionId.size()); dstMesh++)
        {
          for(IndexType transitionNode = 0; transitionNode < localMeshes[srcMeshIndex].transitionNodesCount[dstMesh]; transitionNode++)
          {
            nodeInfo[localMeshes[srcMeshIndex].transitionNodesGlobalIndices[dstMesh][transitionNode]].tmpIndex = transitionNode;
          }
          for(IndexType sharedCell = 0; sharedCell < localMeshes[srcMeshIndex].sharedCellsCount[dstMesh]; sharedCell++)
          {
            for(IndexType i = 0; i < 4; i++)
            {
              localMeshes[srcMeshIndex].sharedCellsTransitionIndices[dstMesh][sharedCell * 4 + i] =
                nodeInfo[localMeshes[srcMeshIndex].sharedCellsGlobalIndices[dstMesh][sharedCell * 4 + i]].tmpIndex;
            }
          }
        }
      }
    }


    void ComputeLocalNodeIndices()
    {
      IndexType nodeGlobalIndicesPoolSize = 0;
      for(IndexType nodeIndex = 0; nodeIndex < nodesCount; nodeIndex++)
      {
        for(IndexType incidentRegion = 0; incidentRegion < nodeInfo[nodeIndex].incidentRegionsCount; incidentRegion++)
        {
          IndexType incidentRegionIndex = nodeInfo[nodeIndex].incindentRegionId[incidentRegion];
          localMeshes[incidentRegionIndex].nodesCount++;
          nodeGlobalIndicesPoolSize++;
        }
      }

      nodeGlobalIndicesPool = new IndexType[nodeGlobalIndicesPoolSize];

      IndexType offset = 0;
      for(IndexType meshIndex = 0; meshIndex < meshesCount; meshIndex++)
      {
        localMeshes[meshIndex].nodeGlobalIndices = nodeGlobalIndicesPool + offset;
        offset += localMeshes[meshIndex].nodesCount;
      }

      for(IndexType nodeIndex = 0; nodeIndex < nodesCount; nodeIndex++)
      {
        for(IndexType incidentRegion = 0; incidentRegion < nodeInfo[nodeIndex].incidentRegionsCount; incidentRegion++)
        {
          IndexType incidentRegionIndex = nodeInfo[nodeIndex].incindentRegionId[incidentRegion];
          IndexType nodeLocalIndex = nodeInfo[nodeIndex].localIndex[incidentRegion];
          localMeshes[incidentRegionIndex].nodeGlobalIndices[nodeLocalIndex] = nodeIndex;
        }
      }
    }

    void ComputeLocalSubmeshes(IndexType *subMeshNodesCount, IndexType subMeshesCount)
    {
      for(IndexType meshIndex = 0; meshIndex < meshesCount; meshIndex++)
      {
        localMeshes[meshIndex].subMeshesCount = subMeshesCount;
        localMeshes[meshIndex].subMeshNodesCount = new IndexType[subMeshesCount];
        for(IndexType subMeshIndex = 0; subMeshIndex < subMeshesCount; subMeshIndex++)
        {
          localMeshes[meshIndex].subMeshNodesCount[subMeshIndex] = 0;
        }
      }

      bool ok = 1;
      IndexType currSubMeshIndex = 0;
      IndexType subMeshNodesRemain = subMeshNodesCount[currSubMeshIndex];
      for(IndexType nodeIndex = 0; (nodeIndex < nodesCount) && ok; nodeIndex++)
      {
        while((subMeshNodesRemain == 0) && (currSubMeshIndex + 1 < subMeshesCount))
        {
          currSubMeshIndex++;
          subMeshNodesRemain = subMeshNodesCount[currSubMeshIndex];
        }
        if(subMeshNodesRemain == 0)
        {
          ok = 0;
          break;
        }

        for(IndexType incidentRegion = 0; incidentRegion < nodeInfo[nodeIndex].incidentRegionsCount; incidentRegion++)
        {
          IndexType incidentRegionIndex = nodeInfo[nodeIndex].incindentRegionId[incidentRegion];
          localMeshes[incidentRegionIndex].subMeshNodesCount[currSubMeshIndex]++;
        }
        subMeshNodesRemain--;
      }

      if(ok == 0)
      {
        std::cout << "Submesh nodes count mismatch" << std::endl;
      }
    }

    void ComputeLocalContactFaces(ContactFace *contactFaces, IndexType *contactFacesCount, IndexType contactTypesCount)
    {
      for(IndexType meshIndex = 0; meshIndex < meshesCount; meshIndex++)
      {
        localMeshes[meshIndex].contactFaces = 0;
        localMeshes[meshIndex].contactTypesCount = contactTypesCount;
        localMeshes[meshIndex].contactFacesCount = new IndexType[contactTypesCount];

        for(IndexType contactTypeIndex = 0; contactTypeIndex < contactTypesCount; contactTypeIndex++)
        {
          localMeshes[meshIndex].contactFacesCount[contactTypeIndex] = 0;
        }
      }

      IndexType globalContactFacesCount = 0;
      for(IndexType contactTypeIndex = 0; contactTypeIndex < contactTypesCount; contactTypeIndex++)
      {
        globalContactFacesCount += contactFacesCount[contactTypeIndex];
      }

      IndexType contactFacesPoolSize = 0;

      IndexType currContactType = 0;
      IndexType currContactTypeEnd = contactFacesCount[currContactType];
      for(IndexType contactFaceIndex = 0; contactFaceIndex < globalContactFacesCount; contactFaceIndex++)
      {
        for(;(contactFaceIndex >= currContactTypeEnd) && (currContactType < contactTypesCount);
            currContactTypeEnd += contactFacesCount[++currContactType]);

        IndexType referenceNode = contactFaces[contactFaceIndex].faces[0].nodes[0];

        for(IndexType incidentRegion = 0; incidentRegion < nodeInfo[referenceNode].incidentRegionsCount; incidentRegion++)
        {
          IndexType incidentRegionIndex = nodeInfo[referenceNode].incindentRegionId[incidentRegion];

          bool fineFace = 1;
          IndexType localNodeIndices[2][3];
          for(IndexType faceIndex = 0; faceIndex < 2; faceIndex++)
          {
            for(IndexType nodeIndex = 0; nodeIndex < 3; nodeIndex++)
            {
              localNodeIndices[faceIndex][nodeIndex] =
                GetNodeLocalIndex(incidentRegionIndex, contactFaces[contactFaceIndex].faces[faceIndex].nodes[nodeIndex]);

              if(localNodeIndices[faceIndex][nodeIndex] == -1)
              {
                fineFace = 0;
              }
            }
          }

          if(fineFace)
          {
            localMeshes[incidentRegionIndex].contactFacesCount[currContactType]++;
            contactFacesPoolSize++;
          }
        }
      }

      localContactFacesPool = new ContactFace[contactFacesPoolSize];
      IndexType offset = 0;
      for(IndexType meshIndex = 0; meshIndex < meshesCount; meshIndex++)
      {
        localMeshes[meshIndex].contactFaces = localContactFacesPool + offset;
        IndexType localContactFacesCount = 0;
        for(IndexType contactTypeIndex = 0; contactTypeIndex < contactTypesCount; contactTypeIndex++)
        {
          localContactFacesCount += localMeshes[meshIndex].contactFacesCount[contactTypeIndex];
          localMeshes[meshIndex].contactFacesCount[contactTypeIndex] = 0;
        }
        localMeshes[meshIndex].totalContactFacesCount = 0;
        offset += localContactFacesCount;
      }

      currContactType = 0;
      currContactTypeEnd = contactFacesCount[currContactType];
      for(IndexType contactFaceIndex = 0; contactFaceIndex < globalContactFacesCount; contactFaceIndex++)
      {
        for(;(contactFaceIndex >= currContactTypeEnd) && (currContactType < contactTypesCount);
            currContactTypeEnd += contactFacesCount[++currContactType]);

        IndexType referenceNode = contactFaces[contactFaceIndex].faces[0].nodes[0];

        for(IndexType incidentRegion = 0; incidentRegion < nodeInfo[referenceNode].incidentRegionsCount; incidentRegion++)
        {
          IndexType incidentRegionIndex = nodeInfo[referenceNode].incindentRegionId[incidentRegion];

          IndexType localNodeIndices[2][3];
          bool fineFace = 1;
          for(IndexType faceIndex = 0; faceIndex < 2; faceIndex++)
          {
            for(IndexType nodeIndex = 0; nodeIndex < 3; nodeIndex++)
            {
              localNodeIndices[faceIndex][nodeIndex] =
                GetNodeLocalIndex(incidentRegionIndex, contactFaces[contactFaceIndex].faces[faceIndex].nodes[nodeIndex]);
              if(localNodeIndices[faceIndex][nodeIndex] == -1)
              {
                fineFace = 0;
              }
            }
          }

          if(fineFace)
          {
            for(IndexType faceIndex = 0; faceIndex < 2; faceIndex++)
            {
              for(IndexType nodeIndex = 0; nodeIndex < 3; nodeIndex++)
              {
                localMeshes[incidentRegionIndex].contactFaces[localMeshes[incidentRegionIndex].totalContactFacesCount].faces[faceIndex].nodes[nodeIndex] =
                  GetNodeLocalIndex(incidentRegionIndex, contactFaces[contactFaceIndex].faces[faceIndex].nodes[nodeIndex]);
              }
            }
            localMeshes[incidentRegionIndex].contactFacesCount[currContactType]++;
            localMeshes[incidentRegionIndex].totalContactFacesCount++;
          }
        }
      }
    }

    void ComputeLocalBoundaryFaces(BoundaryFace *boundaryFaces, IndexType *boundaryFacesCount, IndexType boundaryTypesCount)
    {
      for(IndexType meshIndex = 0; meshIndex < meshesCount; meshIndex++)
      {
        localMeshes[meshIndex].boundaryFaces = 0;
        localMeshes[meshIndex].boundaryTypesCount = boundaryTypesCount;
        localMeshes[meshIndex].boundaryFacesCount = new IndexType[boundaryTypesCount];
        for(IndexType boundaryTypeIndex = 0; boundaryTypeIndex < boundaryTypesCount; boundaryTypeIndex++)
        {
          localMeshes[meshIndex].boundaryFacesCount[boundaryTypeIndex] = 0;
        }
      }

      IndexType globalBoundaryFacesCount = 0;
      for(IndexType boundaryTypeIndex = 0; boundaryTypeIndex < boundaryTypesCount; boundaryTypeIndex++)
      {
        globalBoundaryFacesCount += boundaryFacesCount[boundaryTypeIndex];
      }

      IndexType boundaryFacesPoolSize = 0;

      IndexType currBoundaryType = 0;
      IndexType currBoundaryTypeEnd = boundaryFacesCount[currBoundaryType];
      for(IndexType boundaryFaceIndex = 0; boundaryFaceIndex < globalBoundaryFacesCount; boundaryFaceIndex++)
      {
        for(;(boundaryFaceIndex >= currBoundaryTypeEnd) && (currBoundaryType < boundaryTypesCount);
            currBoundaryTypeEnd += boundaryFacesCount[++currBoundaryType]);

        IndexType referenceNode = boundaryFaces[boundaryFaceIndex].nodes[0];

        for(IndexType incidentRegion = 0; incidentRegion < nodeInfo[referenceNode].incidentRegionsCount; incidentRegion++)
        {
          IndexType incidentRegionIndex = nodeInfo[referenceNode].incindentRegionId[incidentRegion];

          IndexType localNodeIndices[3];

          bool fineFace = 1;
          for(IndexType nodeIndex = 0; nodeIndex < 3; nodeIndex++)
          {
            localNodeIndices[nodeIndex] =
              GetNodeLocalIndex(incidentRegionIndex, boundaryFaces[boundaryFaceIndex].nodes[nodeIndex]);
            if(localNodeIndices[nodeIndex] == -1)
            {
              fineFace = 0;
            }
          }

          if(fineFace)
          {
            localMeshes[incidentRegionIndex].boundaryFacesCount[currBoundaryType]++;
            boundaryFacesPoolSize++;
          }
        }
      }

      localBoundaryFacesPool = new BoundaryFace[boundaryFacesPoolSize];
      IndexType offset = 0;
      for(IndexType meshIndex = 0; meshIndex < meshesCount; meshIndex++)
      {
        localMeshes[meshIndex].boundaryFaces = localBoundaryFacesPool + offset;
        IndexType localFacesCount = 0;
        for(IndexType boundaryTypeIndex = 0; boundaryTypeIndex < boundaryTypesCount; boundaryTypeIndex++)
        {
          localFacesCount += localMeshes[meshIndex].boundaryFacesCount[boundaryTypeIndex];
          localMeshes[meshIndex].boundaryFacesCount[boundaryTypeIndex] = 0;
        }
        localMeshes[meshIndex].totalBoundaryFacesCount = 0;
        offset += localFacesCount;
      }

      currBoundaryType = 0;
      currBoundaryTypeEnd = boundaryFacesCount[currBoundaryType];

      for(IndexType boundaryFaceIndex = 0; boundaryFaceIndex < globalBoundaryFacesCount; boundaryFaceIndex++)
      {
        for(;(boundaryFaceIndex >= currBoundaryTypeEnd) && (currBoundaryType < boundaryTypesCount);
            currBoundaryTypeEnd += boundaryFacesCount[++currBoundaryType]);

        IndexType referenceNode = boundaryFaces[boundaryFaceIndex].nodes[0];

        for(IndexType incidentRegion = 0; incidentRegion < nodeInfo[referenceNode].incidentRegionsCount; incidentRegion++)
        {
          IndexType incidentRegionIndex = nodeInfo[referenceNode].incindentRegionId[incidentRegion];

          IndexType localNodeIndices[3];
          bool fineFace = 1;
          for(IndexType nodeIndex = 0; nodeIndex < 3; nodeIndex++)
          {
            localNodeIndices[nodeIndex] =
              GetNodeLocalIndex(incidentRegionIndex, boundaryFaces[boundaryFaceIndex].nodes[nodeIndex]);
            if(localNodeIndices[nodeIndex] == -1)
            {
              fineFace = 0;
            }
          }

          if(fineFace)
          {
            for(IndexType nodeIndex = 0; nodeIndex < 3; nodeIndex++)
            {
              localMeshes[incidentRegionIndex].boundaryFaces[localMeshes[incidentRegionIndex].totalBoundaryFacesCount].nodes[nodeIndex] =
                GetNodeLocalIndex(incidentRegionIndex, boundaryFaces[boundaryFaceIndex].nodes[nodeIndex]);
            }
            localMeshes[incidentRegionIndex].boundaryFacesCount[currBoundaryType]++;
            localMeshes[incidentRegionIndex].totalBoundaryFacesCount++;
          }
        }
      }
    }

    IndexType *expandedCellIndices;
    IndexType *expandedCellRegionId;
    IndexType normalCellsCount;
    IndexType expandedCellsCount;
    IndexType nodesCount;
    IndexType meshesCount;

/*    SubMeshInfo *subMeshes;
    IndexType subMeshesCount;*/

    struct LocalMesh
    {
      IndexType nodesCount;
      IndexType *nodeGlobalIndices;

      IndexType cellsCount;
      IndexType *cellGlobalIndices;

      std::vector<IndexType> destRegionId;

      std::vector<IndexType> transitionNodesCount;
      IndexType **transitionNodesGlobalIndices;

      std::vector<IndexType> sharedCellsCount;
      IndexType **sharedCellsGlobalIndices;
      IndexType **sharedCellsTransitionIndices;

      IndexType *subMeshNodesCount;
      IndexType subMeshesCount;

      ContactFace  *contactFaces;
      IndexType    *contactFacesCount;
      IndexType     contactTypesCount;
      IndexType     totalContactFacesCount;


      BoundaryFace *boundaryFaces;
      IndexType    *boundaryFacesCount;
      IndexType     boundaryTypesCount;
      IndexType     totalBoundaryFacesCount;
    };
    LocalMesh *localMeshes;

    IndexType *nodeGlobalIndicesPool;
    IndexType *cellGlobalIndicesPool;
    IndexType *sharedCellsGlobalIndicesPool;
    IndexType *sharedCellsTransitionIndicesPool;
    IndexType *transitionNodesGlobalIndicesPool;
    ContactFace   *localContactFacesPool;
    BoundaryFace  *localBoundaryFacesPool;


    struct NodeInfo
    {
      IndexType incidentRegionsCount;
      IndexType *incindentRegionId;
      IndexType *localIndex;
      IndexType tmpIndex;
    };
    IndexType *incidentRegionIdPool;
    IndexType *localIndexPool;

    NodeInfo *nodeInfo;

  };
} //namespace swift
