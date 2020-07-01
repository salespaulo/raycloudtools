// Copyright (c) 2020
// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
// ABN 41 687 119 230
//
// Author: Thomas Lowe
#ifndef RAYLIB_RAYCONCAVEHULL_H
#define RAYLIB_RAYCONCAVEHULL_H

#include "raylib/raylibconfig.h"

#include "rayutils.h"
#include <set>

#if RAYLIB_WITH_QHULL
namespace ray
{
class RAYLIB_EXPORT ConcaveHull
{
public:
  ConcaveHull(const std::vector<Eigen::Vector3d> &points);

  void growInwards(double maxCurvature);
  void growOutwards(double maxCurvature);
  void growInDirection(double maxCurvature, const Eigen::Vector3d &dir);
  void growUpwards(double maxCurvature) { growInDirection(maxCurvature, Eigen::Vector3d(0, 0, 1)); }
  void growTopDown(double maxCurvature) { growInDirection(maxCurvature, Eigen::Vector3d(0, 0, -1)); }


  class RAYLIB_EXPORT SurfaceFace
  {
  public:
    SurfaceFace() { triangle = -1; }
    int tetrahedron;
    int triangle;
    double curvature;
  };

  class RAYLIB_EXPORT Edge
  {
  public:
    Edge() {}
    Edge(int v1, int v2)
    {
      vertices[0] = v1;
      vertices[1] = v2;
      has_had_face = false;
    }
    int vertices[2];
    bool has_had_face;
  };
  class RAYLIB_EXPORT Triangle
  {
  public:
    Triangle()
    {
      vertices[0] = vertices[1] = vertices[2] = -1;
      edges = Eigen::Vector3i(-1, -1, -1);
      is_surface = false;
      used = false;
    }
    bool valid() { return vertices[0] != -1; }
    bool is_surface;
    bool used;
    Eigen::Vector3i vertices;
    Eigen::Vector3i edges;
    int tetrahedra[2];
    SurfaceFace surface_face_cached;
  };
  class RAYLIB_EXPORT Tetrahedron
  {
  public:
    Tetrahedron()
    {
      vertices[0] = vertices[1] = vertices[2] = vertices[3] = -1;
      seen = false;
      id = -1;
    }
    bool valid() { return vertices[0] != -1; }
    int vertices[4];
    int triangles[4];
    int neighbours[4];
    int id;
    bool seen;
  };
  bool insideTetrahedron(const Eigen::Vector3d &pos, const Tetrahedron &tetra)
  {
    Eigen::Vector3d mid(0, 0, 0);
    if (tetra.vertices[0] == -1 || tetra.vertices[1] == -1 || tetra.vertices[2] == -1 ||
        tetra.vertices[3] == -1)  // an outer tetrahedron
      return false;
    for (int j = 0; j < 4; j++) mid += vertices[tetra.vertices[j]] / 4.0;
    for (int i = 0; i < 4; i++)
    {
      Eigen::Vector3d vs[3];
      for (int j = 0; j < 3; j++) vs[j] = vertices[triangles[tetra.triangles[i]].vertices[j]];
      Eigen::Vector3d normal = (vs[1] - vs[0]).cross(vs[2] - vs[0]);
      if ((pos - vs[0]).dot(normal) * (mid - vs[0]).dot(normal) < 0)
        return false;
    }
    return true;
  }
  std::vector<bool> vertex_on_surface;
  std::vector<Eigen::Vector3d> vertices;
  std::vector<Edge> edges;

  std::vector<Triangle> triangles;
  std::vector<Tetrahedron> tetrahedra;
  Eigen::Vector3d centre;

  class RAYLIB_EXPORT FaceComp
  {
  public:
    bool operator()(const SurfaceFace &lhs, const SurfaceFace &rhs) const
    {
      if (lhs.curvature == rhs.curvature)
      {
        if (lhs.triangle == rhs.triangle)
          return lhs.tetrahedron < rhs.tetrahedron;
        return lhs.triangle < rhs.triangle;
      }
      return lhs.curvature < rhs.curvature;
    }
  };
  std::set<SurfaceFace, FaceComp> surface;

protected:
  void growSurface(double maxCurvature);
  bool growFront(double maxCurvature);
  double circumcurvature(const ConcaveHull::Tetrahedron &tetra, int triangleID);
  void growOutwards(const ConcaveHull::Tetrahedron &tetra, double maxCurvature);
};
}  // namespace ray

#endif

#endif  // RAYLIB_RAYCONCAVEHULL_H
