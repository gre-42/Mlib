#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <list>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <Recast.h>
#pragma clang diagnostic pop
#include <list>
#include <optional>

class InputGeom;

namespace Mlib {

struct LocalizedNavmeshNode {
    FixedArray<float, 3> position;
    dtPolyRef polyRef;
};

class Sample_SoloMesh
{
private:
    bool m_keepInterResults;
    float m_totalBuildTimeMs;

    unsigned char* m_triareas;
    rcHeightfield* m_solid;
    rcCompactHeightfield* m_chf;
    rcContourSet* m_cset;
    rcPolyMesh* m_pmesh;
    rcPolyMeshDetail* m_dmesh;

    const InputGeom* m_geom;
    dtNavMesh* m_navMesh;
    dtNavMeshQuery* m_navQuery;
    rcContext* m_ctx;

    rcConfig m_cfg;

    dtQueryFilter m_filter;

    void cleanup();

public:
    bool m_filterLowHangingObstacles;
    bool m_filterLedgeSpans;
    bool m_filterWalkableLowHeightSpans;

    float m_cellSize;
    float m_cellHeight;
    float m_agentHeight;
    float m_agentRadius;
    float m_agentMaxClimb;
    float m_agentMaxSlope;
    float m_regionMinSize;
    float m_regionMergeSize;
    float m_edgeMaxLen;
    float m_edgeMaxError;
    float m_vertsPerPoly;
    float m_detailSampleDist;
    float m_detailSampleMaxError;
    int m_partitionType;
    FixedArray<float, 3> m_polyPickExtent;

    Sample_SoloMesh(
        rcContext& ctx,
        const InputGeom& geom);
    ~Sample_SoloMesh();
    
    void resetCommonSettings();
    bool build();
    std::optional<LocalizedNavmeshNode> closest_point_on_navmesh(const FixedArray<float, 3>& point) const;
    std::list<FixedArray<float, 3>> shortest_path(
        const LocalizedNavmeshNode& start,
        const LocalizedNavmeshNode& end,
        float step_size) const;

private:
    // Explicitly disabled copy constructor and copy assignment operator.
    Sample_SoloMesh(const Sample_SoloMesh&) = delete;
    Sample_SoloMesh& operator=(const Sample_SoloMesh&) = delete;
};

}
