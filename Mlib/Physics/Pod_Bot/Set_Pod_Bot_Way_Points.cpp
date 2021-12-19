#include "Set_Pod_Bot_Way_Points.hpp"
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <pod_bot/bot_globals.h>

using namespace Mlib;

void InitWaypointTypes();

void Mlib::set_pod_bot_way_points(
   const SceneNode& node,
   const std::map<WayPointLocation, PointsAndAdjacency<float, 3>>& all_waypoints)
{
   TransformationMatrix<float, 3> m = node.absolute_model_matrix();
   float scale = m.get_scale();

   g_bMapInitialised = FALSE;
   g_bRecalcVis = FALSE;
   g_fTimeDisplayVisTableMsg = 0;
   g_bWaypointsSaved = FALSE;

   for (int i = 0; i < g_iNumWaypoints; ++i) {
      delete paths[i];
      paths[i] = nullptr;
   }

   g_iNumWaypoints = 0;
   for (const auto& w : all_waypoints) {
      for (int i = 0; i < (int)w.second.points.size(); ++i) {
         if (g_iNumWaypoints == MAX_WAYPOINTS) {
            throw std::runtime_error("MAX_WAYPOINTS exceeded");
         }
         const auto& column = w.second.adjacency.column(i);
         auto cit = column.begin();
         auto& path = *new PATH;
         paths[g_iNumWaypoints] = &path;
         path.iPathNumber = g_iNumWaypoints;
         path.flags = 0;
         path.origin = m.transform(w.second.points[i]);
         path.Radius = 1.f;
         path.fcampstartx = 0.f;
         path.fcampstarty = 0.f;
         path.fcampendx = 0.f;
         path.fcampendy = 0.f;
         path.index[0] = cit == column.end() ? -1 : (cit++)->first;
         path.index[1] = cit == column.end() ? -1 : (cit++)->first;
         path.index[2] = cit == column.end() ? -1 : (cit++)->first;
         path.index[3] = cit == column.end() ? -1 : (cit++)->first;
         path.index[4] = cit == column.end() ? -1 : (cit++)->first;
         path.index[5] = cit == column.end() ? -1 : (cit++)->first;
         path.index[6] = cit == column.end() ? -1 : (cit++)->first;
         path.index[7] = cit == column.end() ? -1 : (cit++)->first;
         if (cit != column.end()) {
            std::cerr << "WARNING: Waypoint has too many neighbors" << std::endl;
         }
         cit = column.begin();
         path.distance[0] = cit == column.end() ? -1 : (cit++)->second * scale;
         path.distance[1] = cit == column.end() ? -1 : (cit++)->second * scale;
         path.distance[2] = cit == column.end() ? -1 : (cit++)->second * scale;
         path.distance[3] = cit == column.end() ? -1 : (cit++)->second * scale;
         path.distance[4] = cit == column.end() ? -1 : (cit++)->second * scale;
         path.distance[5] = cit == column.end() ? -1 : (cit++)->second * scale;
         path.distance[6] = cit == column.end() ? -1 : (cit++)->second * scale;
         path.distance[7] = cit == column.end() ? -1 : (cit++)->second * scale;
         path.connectflag[0] = 0;
         path.connectflag[1] = 0;
         path.connectflag[2] = 0;
         path.connectflag[3] = 0;
         path.connectflag[4] = 0;
         path.connectflag[5] = 0;
         path.connectflag[6] = 0;
         path.connectflag[7] = 0;
         path.vecConnectVel[0] = g_vecZero;
         path.vecConnectVel[1] = g_vecZero;
         path.vecConnectVel[2] = g_vecZero;
         path.vecConnectVel[3] = g_vecZero;
         path.vecConnectVel[4] = g_vecZero;
         path.vecConnectVel[5] = g_vecZero;
         path.vecConnectVel[6] = g_vecZero;
         path.vecConnectVel[7] = g_vecZero;
         ++g_iNumWaypoints;
      }
   }

   for (int index = 0; index < g_iNumWaypoints; index++)
      paths[index]->next = paths[index + 1];
   if (g_iNumWaypoints > 0)
      paths[g_iNumWaypoints - 1]->next = NULL;

   InitWaypointTypes ();
   InitPathMatrix ();

   g_bWaypointsChanged = FALSE;
}
