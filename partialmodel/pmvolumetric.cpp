/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "pmvolumetric.h"

PMVolumetric::PMVolumetric(): PartialModelBase()
{
    voxelResolution = 0.1; // default resolution 0.1 m
    freeSpace = false;
    
    minUnknown = 1;
    thereIsUnmarkVoxelsInNBV = true;
    stopCriteria = false;
}





bool PMVolumetric::init()
{
    PartialModelBase::init();
    
    std::string config_file(configFolder);
    config_file.append("/");
    config_file.append("partialModelConfig.ini");
  
    mrpt::utils::CConfigFile parser;
    ASSERT_FILE_EXISTS_(config_file);
    parser.setFileName(config_file);
    
    voxelResolution = parser.read_double("volumetric", "resolution", -1, true);
    
    weight = parser.read_double("volumetric", "weight", 0.5, true);
    
    freeSpace = parser.read_bool("volumetric", "freeSpace", false, true);
    
    collisionGap = parser.read_double("volumetric", "collisionGap", 0.1, true);
    
    minUnknown = parser.read_int("volumetric", "minUnknown", 100);
    
    minOverlap = parser.read_int("volumetric", "minOverlap", 1); 
    
    if(freeSpace){
      x_free_1 = parser.read_double("freeSpaceCoord", "x1", -1, true);
      x_free_2 = parser.read_double("freeSpaceCoord", "x2", -1, true);
      
      y_free_1 = parser.read_double("freeSpaceCoord", "y1", -1, true);
      y_free_2 = parser.read_double("freeSpaceCoord", "y2", -1, true);
      
      z_free_1 = parser.read_double("freeSpaceCoord", "z1", -1, true);
      z_free_2 = parser.read_double("freeSpaceCoord", "z2", -1, true);
    }
    
    maxRange = parser.read_double( "volumetric", "maxRange", -1, true);
    
    std::cout << "---------- Volumetric configuration -------" << std::endl;
    std::cout << "Voxel reslution:" << voxelResolution << std::endl;
    std::cout << "Weight:" << weight << std::endl;
    std::cout << "Free space:" << freeSpace << std::endl;
    std::cout << "Collision gap:" << collisionGap << std::endl;
    std::cout << "Min unknown:" << minUnknown << std::endl;
    std::cout << "Min overlap:" << minOverlap << std::endl;
    std::cout << "max range:" << maxRange << std::endl;
    
    float x1, x2, y1, y2, z1, z2;
    x1 = parser.read_double("objectCapsule", "x1", 0, true);
    x2 = parser.read_double("objectCapsule", "x2", 0, true);
    y1 = parser.read_double("objectCapsule", "y1", 0, true);
    y2 = parser.read_double("objectCapsule", "y2", 0, true);
    z1 = parser.read_double("objectCapsule", "z1", 0, true);
    z2 = parser.read_double("objectCapsule", "z2", 0, true);
    
    point3d obj_min(x1,y1,z1);
    ObjectBBxMin = obj_min;
    
    point3d obj_max(x2,y2,z2);
    ObjectBBxMax = obj_max;
    
    x1 = parser.read_double("sceneCapsule", "x1", 0, true);
    x2 = parser.read_double("sceneCapsule", "x2", 0, true);
    y1 = parser.read_double("sceneCapsule", "y1", 0, true);
    y2 = parser.read_double("sceneCapsule", "y2", 0, true);
    z1 = parser.read_double("sceneCapsule", "z1", 0, true);
    z2 = parser.read_double("sceneCapsule", "z2", 0, true);
    
    point3d sce_min(x1,y1,z1);
    SceneBBxMin = sce_min;
    point3d sce_max(x2,y2,z2);
    SceneBBxMax = sce_max;
       
    return true;
}


bool PMVolumetric::collisionFree(float x, float y, float z)
{
  std::cout << "to be implemented" << std::endl;
  return true;
}

//TODO
void PMVolumetric::saveEvaluations()
{
//   //PartialModelBase::saveEvaluations();
//   //saveEvaluationResultVector(evaluations, evaluationsFile);
//   evals.saveVector(evaluationsFile);
}
// 

void PMVolumetric::setMinimunNumberOfUnmarkVoxels(int n)
{
  minUnknown = n;
}

bool PMVolumetric::stopCriteriaReached()
{
  return stopCriteria;
}


void PMVolumetric::getVoxelVertices(point3d_list& vertices, const point3d center, const double resolution)
{
  //TEST
  vertices.clear();
  
  double x, y, z;
  double offset = resolution/2;
  
  // ---- vertex 1 ----
  x = center.x() - offset;
  y = center.y() - offset;
  z = center.z() - offset;
  point3d p1(x,y,z);
  vertices.push_back(p1);
  
  // ---- vertex 2 ----
  x = center.x() + offset;
  y = center.y() - offset;
  z = center.z() - offset;
  point3d p2(x,y,z);
  vertices.push_back(p2);
  
  // ---- vertex 3 ----
  x = center.x() + offset;
  y = center.y() - offset;
  z = center.z() + offset;
  point3d p3(x,y,z);
  vertices.push_back(p3);
  
   // ---- vertex 4 ----
  x = center.x() - offset;
  y = center.y() - offset;
  z = center.z() + offset;
  point3d p4(x,y,z);
  vertices.push_back(p4);
  
   // ---- vertex 5 ----
  x = center.x() - offset;
  y = center.y() + offset;
  z = center.z() - offset;
  point3d p5(x,y,z);
  vertices.push_back(p5);
  
   // ---- vertex 6 ----
  x = center.x() + offset;
  y = center.y() + offset;
  z = center.z() - offset;
  point3d p6(x,y,z);
  vertices.push_back(p6);
  
   // ---- vertex 7 ----
  x = center.x() + offset;
  y = center.y() + offset;
  z = center.z() + offset;
  point3d p7(x,y,z);
  vertices.push_back(p7);
  
   // ---- vertex 8 ----
  x = center.x() - offset;
  y = center.y() + offset;
  z = center.z() + offset;
  point3d p8(x,y,z);
  vertices.push_back(p8);  
}


void PMVolumetric::getTrianglesOfVoxel(vpTriangleList& tris, const point3d center, const double resolution)
{
  vpVertex vertex;
  std::vector<vpVertex> vertices(8);
  double x, y, z;
  double offset = resolution/2;
  
  // ---- vertex 1 ----
  x = center.x() - offset;
  y = center.y() - offset;
  z = center.z() - offset;
  vertex.setCoordinates(x,y,z);
  vertices[0] = vertex;
  
  // ---- vertex 2 ----
  x = center.x() + offset;
  y = center.y() - offset;
  z = center.z() - offset;
  vertex.setCoordinates(x,y,z);
  vertices[1] = vertex;
  
  // ---- vertex 3 ----
  x = center.x() + offset;
  y = center.y() - offset;
  z = center.z() + offset;
  vertex.setCoordinates(x,y,z);
  vertices[2] = vertex;
  
   // ---- vertex 4 ----
  x = center.x() - offset;
  y = center.y() - offset;
  z = center.z() + offset;
  vertex.setCoordinates(x,y,z);
  vertices[3] = vertex;
  
   // ---- vertex 5 ----
  x = center.x() - offset;
  y = center.y() + offset;
  z = center.z() - offset;
  vertex.setCoordinates(x,y,z);
  vertices[4] = vertex;
  
   // ---- vertex 6 ----
  x = center.x() + offset;
  y = center.y() + offset;
  z = center.z() - offset;
  vertex.setCoordinates(x,y,z);
  vertices[5] = vertex;
  
   // ---- vertex 7 ----
  x = center.x() + offset;
  y = center.y() + offset;
  z = center.z() + offset;
  vertex.setCoordinates(x,y,z);
  vertices[6] = vertex;
  
   // ---- vertex 8 ----
  x = center.x() - offset;
  y = center.y() + offset;
  z = center.z() + offset;
  vertex.setCoordinates(x,y,z);
  vertices[7] = vertex; 
  
  vpTriangle t;
  tris.clear();
  
  t.a = vertices[0];
  t.b = vertices[1];
  t.c = vertices[2];
  tris.push_back(t);
  
  t.a = vertices[0];
  t.b = vertices[2];
  t.c = vertices[3];
  tris.push_back(t);
  
  t.a = vertices[0];
  t.b = vertices[1];
  t.c = vertices[4];
  tris.push_back(t);
  
  t.a = vertices[1];
  t.b = vertices[4];
  t.c = vertices[5];
  tris.push_back(t);
  
  t.a = vertices[4];
  t.b = vertices[5];
  t.c = vertices[6];
  tris.push_back(t);
  
  t.a = vertices[4];
  t.b = vertices[6];
  t.c = vertices[7];
  tris.push_back(t);
  
  t.a = vertices[0];
  t.b = vertices[4];
  t.c = vertices[7];
  tris.push_back(t);
  
  t.a = vertices[0];
  t.b = vertices[3];
  t.c = vertices[7];
  tris.push_back(t);
  
  t.a = vertices[2];
  t.b = vertices[6];
  t.c = vertices[7];
  tris.push_back(t);
  
  t.a = vertices[2];
  t.b = vertices[7];
  t.c = vertices[3];
  tris.push_back(t);
  
  t.a = vertices[1];
  t.b = vertices[5];
  t.c = vertices[6];
  tris.push_back(t);
  
  t.a = vertices[1];
  t.b = vertices[6];
  t.c = vertices[2];
  tris.push_back(t);
  
}
