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


#include "partialmodelbase.h"


PartialModelBase::PartialModelBase():
colorOrigin(255,255,0),
colorRed(255,0,0),
colorBlue(0,0,255),
colorYellow(255,255,0),
colorCian(0,255,255),
colorOrange(255,165,0),
colorGray(200,200,200)
{
  scanOrigin = new octomap::point3d (0,0,0);
  scanCloudOrigins = new octomap::Pointcloud();
  scanCloud = new octomap::Pointcloud();
  
  colorOccupied = colorBlue;
  colorUnmark = colorOrange;
  colorTouchedOccupied = colorCian;
  colorTouchedUnkmark = colorRed;
  
  object_points_filename.assign("object_pts.dat");
}


bool PartialModelBase::init()
{
  std::cout << "Reading parameters" << std::endl;
 
  std::string config_file(configFolder);
  config_file.append("/");
  config_file.append("partialModelConfig.ini");
  
  mrpt::utils::CConfigFile parser;
  ASSERT_FILE_EXISTS_(config_file);
  parser.setFileName(config_file);

     
  std::cout << "\n------------- Partial Model Configuration ------------" << std::endl;
  
  float x1, x2, y1, y2, z1, z2;
  x1 = parser.read_double("objectCapsule", "x1", 0, true);  
  x2 = parser.read_double("objectCapsule", "x2", 0, true);  
  y1 = parser.read_double("objectCapsule", "y1", 0, true);  
  y2 = parser.read_double("objectCapsule", "y2", 0, true);
  z1 = parser.read_double("objectCapsule", "z1", 0, true);
  z2 = parser.read_double("objectCapsule", "z2", 0, true);
  
  setObjectCapsule(x1,y1,z1,x2,y2,z2);
  
  x1 = parser.read_double("sceneCapsule", "x1", 0, true);
  x2 = parser.read_double("sceneCapsule", "x2", 0, true);
  y1 = parser.read_double("sceneCapsule", "y1", 0, true);
  y2 = parser.read_double("sceneCapsule", "y2", 0, true);
  z1 = parser.read_double("sceneCapsule", "z1", 0, true);
  z2 = parser.read_double("sceneCapsule", "z2", 0, true);
  
  setScene(x1,y1,z1,x2,y2,z2);
  
  evaluationsFile.clear();
  evaluationsFile.assign(dataFolder);
  evaluationsFile.append("/partial_model_evaluations.dat");
  
  minDOV = parser.read_double("sensor", "minDOV", 0, true);
  maxDOV = parser.read_double("sensor", "maxDOV", 50, true);
  
  //rbb_size = parser.read_double(ini_file, "partialModel:rbb_size", 0);
  
  double x = parser.read_double("objectCenter", "x", 0, true);
  double y = parser.read_double("objectCenter", "y", 0, true);
  double z = parser.read_double("objectCenter", "z", 0, true);
  double r = parser.read_double("objectCenter", "radio", 1.0, true);
  point3d p(x,y,z);
  objectSphereCenter = p;
  objectSphereRadius = r;
  objRadius2 = objectSphereRadius;// * objectSphereRadius;
  
  x = parser.read_double("directorRay", "x", 0, true);
  y = parser.read_double("directorRay", "y", 0, true);
  z = parser.read_double("directorRay", "z", 0, true);
  point3d dr(x,y,z);
  directorRay = dr;
 
  std::cout << "Sensor:" << std::endl;
  std::cout << "  Director ray: " << directorRay << std::endl;
  std::cout << "  Minimun DOV: " << minDOV << std::endl;
  std::cout << "  Maximun DOV: " << maxDOV << std::endl;
  
  x = parser.read_double("sensorPose", "x", 0, true);
  y = parser.read_double("sensorPose", "y", 0, true);
  z = parser.read_double("sensorPose", "z", 0, true);
  
  double yaw, pitch, roll;
  yaw = parser.read_double("sensorPose", "yaw", 0, true);
  pitch = parser.read_double("sensorPose", "pitch", 0, true);
  roll = parser.read_double("sensorPose", "roll", 0, true);
  sensorReferenceFramePose.resize(6);
  sensorReferenceFramePose[0] = x;
  sensorReferenceFramePose[1] = y;
  sensorReferenceFramePose[2] = z;
  sensorReferenceFramePose[3] = yaw;
  sensorReferenceFramePose[4] = pitch;
  sensorReferenceFramePose[5] = roll;
  
  std::cout << "Sensor pose: "; PMUtils::printVector(sensorReferenceFramePose); 
  std::cout << "Object Center: " << p << "  radius:" << objectSphereRadius << std::endl;
  
  std::cout << "-------------------------------------" << std::endl;
  return true;
}


void PartialModelBase::setObjectCapsule(double x1, double y1, double z1, double x2, double y2, double z2)
{
 //TODO: validate data
  
  x_cap_1 = x1;
  y_cap_1 = y1;
  z_cap_1 = z1;
  
  x_cap_2 = x2;
  y_cap_2 = y2;
  z_cap_2 = z2;
}


bool PartialModelBase::saveEvaluatedViews(std::string file_name)
{
  return vsSaveViewList(candidateViews, file_name);
}


bool PartialModelBase::saveOnlyNViews(int n, std::string file_name)
{
  std::list<ViewStructure> secondList;
  getNViewsFromList(candidateViews, n, secondList);
  return vsSaveViewList(secondList, file_name);
}


bool PartialModelBase::readCandidateViews(std::string file_name)
{
  candidateViews.clear();
  return vsReadViewList(candidateViews, file_name);
}


void PartialModelBase::setScene(double x1, double y1, double z1, double x2, double y2, double z2)
{
 //TODO: validate data
  
  x_sce_1 = x1;
  y_sce_1 = y1;
  z_sce_1 = z1;
  
  x_sce_2 = x2;
  y_sce_2 = y2;
  z_sce_2 = z2;
}

// void PartialModelBase::setPaintPartialModel(bool value)
// {
//   paintPartialModel = value;
// }


bool PartialModelBase::isInCapsule(point3d point)
{
  if( (point.x() >=  x_cap_1) && (point.x() <= x_cap_2) )
    if (point.y() >= y_cap_1 && point.y() <= y_cap_2 )
      if ( point.z() >= z_cap_1 && point.z() <= z_cap_2 )
	return true;
  
//  std::cout << point.x() << " " << point.y() << " " << point.z() << std::endl;
//  std::cout << "false" << std::endl;
  return false;
}


bool PartialModelBase::isInScene(point3d point)
{
 if( (point.x() >=  x_sce_1) && (point.x() <= x_sce_2) )
    if (point.y() >= y_sce_1 && point.y() <= y_sce_2 )
      if ( point.z() >= z_sce_1 && point.z() <= z_sce_2 )
	return true;
      
  return false;
}




// bool PartialModelBase::readScanFromPCD(std::string file_name, octomap::Pointcloud &cloud)
// {
// //   double x_p, y_p, z_p;
// //   long int readed_points = 0;
// //  // octomap::point3d *point_on_surface;
// //   
// //   pcl::PointCloud<pcl::PointXYZ> points;
// //   pcl::io::loadPCDFile(file_name.c_str(), points);
// //   pcl::PointCloud<pcl::PointXYZ>::iterator it;
// //   
// //   cloud.clear();
// //   
// //   for(it = points.begin(); it != points.end(); it++){
// //     	//point_on_surface = new octomap::point3d(x_p, y_p, z_p);
// //     	if( isnan(it->x) || isnan(it->y) || isnan(it->z) ){
// // 	    std::cout << it->x << " " << it->y << " " << it->z << std::endl;
// // 	} 
// // 	else {
// // 	  cloud.push_back(new octomap::point3d(it->x, it->y, it->z));
// // 	  readed_points ++;
// // 	}
// // 	//delete point_on_surface;
// //   }
// //   
// //   std::cout << "done." << std::endl;
// //   std::cout << "Readed points: "<< readed_points << std::endl;
// //   
// // //   if(!readPointCloudFromFile(origin_file, *scanCloudOrigins))
// // //     return false;
// // //   
//   return true;
// }


bool PartialModelBase::readPointCloudFromDAT(std::string file_name, Pointcloud& cloud)
{
 double x_p, y_p, z_p;
  long int n_points;
  long int readed_points = 0;
  octomap::point3d *point_on_surface;
 
  cloud.clear();
  
  std::ifstream file(file_name.c_str());
  if(file.is_open()){
//    file >> n_points;
    file >> x_p;
    
    while(file.good()){
	
	file >> y_p;
	file >> z_p;
	
	point_on_surface = new octomap::point3d(x_p, y_p, z_p);
	
	cloud.push_back(new octomap::point3d(x_p, y_p, z_p));
	
	readed_points ++;
	delete point_on_surface;
	
	file >> x_p;
    }
    
    file.close();
  } else {
    std::cout << "Unable to open file" << std::endl;
    return false;
  }
  
  std::cout << "done." << std::endl;
  std::cout << "Readed points: "<< readed_points << std::endl;
  
  return true;
}


void PartialModelBase::computeRayFromOriginToPoint(double x, double y, double z, double& i, double& j, double& k)
{
  float hypotenuse;
  float alpha;
  float beta;
  hypotenuse = sqrt( (x*x) + (y*y));
  alpha = asin( y/hypotenuse );
  beta = atan2(z,hypotenuse);
  i = cos(alpha);
  j = sin(alpha);
  k = sin(beta);
}

void PartialModelBase::computeRayFromPointToPoint(BoostMatrix pointA, BoostMatrix pointB, double& i, double& j, double& k)
{
  //TODO: This can be changed to ray = A-B;
  //double hypotenuse;
  //double alpha;
  //double beta;
  //double x,y,z;
  
  //WARNING !!!
  
  // B - A
  i = pointB(0,0) - pointA(0,0);
  j = pointB(1,0) - pointA(1,0);
  k = pointB(2,0) - pointA(2,0);
  
  
  /*
  x = pointB(0,0) - pointA(0,0);
  y = pointB(1,0) - pointA(1,0);
  z = pointB(2,0) - pointA(2,0);
  
  computeRayFromOriginToPoint(x,y,z,i,j,k);*/
}


bool PartialModelBase::registrationLowConstraint(long int n_occupied)
{
  long int registration_threshold;
  registration_threshold = 1;
  
  if (n_occupied >= registration_threshold)
    return true;
  else
    return false;
}


void PartialModelBase::setConfigFolder(std::string folder)
{
  configFolder = folder;
}

void PartialModelBase::setDataFolder(std::string folder)
{
  dataFolder = folder;
}


void PartialModelBase::sortCandidateViews()
{
  orderViewsHighToLow(candidateViews);
}


bool PartialModelBase::isInDOV(point3d origin, point3d point)
{
  float dist;
  dist = point.distance(origin);
  if(dist > minDOV && dist < maxDOV){
    return true;
  } else {
    return false;
  }
}


bool PartialModelBase::rayIntersectObjectSphere(const point3d raydir, const point3d rayorig)
{
  point3d L = rayorig - objectSphereCenter;
  double a = raydir.dot(raydir);
  double b = 2 * raydir.dot(L);
  double c = L.dot(L) - objRadius2;
  
  double discriminant;
  discriminant = b * b - 4 * a * c;
  
  if(discriminant < 0)
    return false;
  
  // TODO check whether the hit sphere is closer than maxDOV
  
  return true;
}

bool PartialModelBase::rayIntersectSphere(const point3d raydir, const point3d rayorig, const point3d sphere_center, double radius)
{
  point3d L = rayorig - sphere_center;
  double a = raydir.dot(raydir);
  double b = 2 * raydir.dot(L);
  double c = L.dot(L) - radius;
  
  double discriminant;
  discriminant = b * b - 4 * a * c;
  
  if(discriminant < 0)
    return false;
  
  // TODO check whether the hit sphere is closer than maxDOV
  
  return true;
}



bool PartialModelBase::poitsToTheObject(ViewStructure& v)
{
   point3d ray(directorRay);
   ray.rotate_IP(v.w[5],v.w[4],v.w[3]);
   
   //std::cout << "rotated director ray" << ray << std::endl;
   
   point3d origin(v.w[0],v.w[1],v.w[2]);
   
   if(rayIntersectObjectSphere(ray, origin)){
//     std::cout << "points to the object :)" << std::endl;
    return true;
   }
   
   return false;
}


bool PartialModelBase::pointsToASphere(ViewStructure& v, double center_x, double center_y, double center_z, double radius)
{
   point3d ray(directorRay);
   ray.rotate_IP(v.w[5],v.w[4],v.w[3]);
   
   point3d origin(v.w[0],v.w[1],v.w[2]);
   
   point3d center(center_x,center_y,center_z);
   
   if(rayIntersectSphere(ray, origin, center, radius)){
    //cout << "points :)" << std::endl;
    return true;
   }
   
   return false;
}



void PartialModelBase::getOBB(double& x1, double& y1, double& z1, double& x2, double& y2, double& z2)
{
  x1 = x_cap_1;
  y1 = y_cap_1;
  z1 = z_cap_1;
  
  x2 = x_cap_2;
  y2 = y_cap_2;
  z2 = z_cap_2;
}


void PartialModelBase::evaluateCandidateViews(ViewList& views)
{
  std::cout << "Evaluating candidate views" << std::endl;
  ViewList::iterator it_v;
  it_v = views.begin();
  ViewStructure temp_view;
  
  while(it_v != views.end()){
    temp_view = *it_v;
    this->evaluateView(temp_view);
    std::cout << temp_view << std::endl;
    *it_v = temp_view;
    it_v ++;
  }
}
