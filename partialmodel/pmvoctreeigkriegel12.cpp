/*
 * Copyright (c) 2017, <copyright holder> <email>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include "pmvoctreeigkriegel12.h"

COctreeKriegel12::COctreeKriegel12(double resolution): COctreeVPL(resolution)
{
  
}


//TODO: To modify this code to integrate even for already touched voxels

double COctreeKriegel12::castRayIG(const point3d& origin, const point3d& directionP, point3d& end, bool ignoreUnknownCells, double maxRange)
{
  double ig_sum = 0;
  
    OcTreeKey current_key;
    if ( !coordToKeyChecked(origin, current_key) ) {
      OCTOMAP_WARNING_STR("Coordinates out of bounds during ray casting");
      return 0;
    }

    ColorOcTreeNode* startingNode = this->search(current_key);
    if (startingNode){
      if (this->isNodeOccupied(startingNode)){
        // Occupied node found at origin 
        // (need to convert from key, since origin does not need to be a voxel center)
        end = this->keyToCoord(current_key);
        return 0;
      }
    } else if(!ignoreUnknownCells){
      OCTOMAP_ERROR_STR("Origin node at " << origin << " for raycasting not found, does the node exist?");
      end = this->keyToCoord(current_key);
      return 0;
    }

    point3d direction = directionP.normalized();
    bool max_range_set = (maxRange > 0.0);

    int step[3]; 
    double tMax[3];
    double tDelta[3];

    for(unsigned int i=0; i < 3; ++i) {
      // compute step direction
      if (direction(i) > 0.0) step[i] =  1;
      else if (direction(i) < 0.0)   step[i] = -1;
      else step[i] = 0;

      // compute tMax, tDelta
      if (step[i] != 0) {
        // corner point of voxel (in direction of ray)
        double voxelBorder = this->keyToCoord(current_key[i]);
        voxelBorder += double(step[i] * this->resolution * 0.5);

        tMax[i] = ( voxelBorder - origin(i) ) / direction(i);
        tDelta[i] = this->resolution / fabs( direction(i) );
      }
      else {
        tMax[i] =  std::numeric_limits<double>::max();
        tDelta[i] = std::numeric_limits<double>::max();
      }
    }

    if (step[0] == 0 && step[1] == 0 && step[2] == 0){
    	OCTOMAP_ERROR("Raycasting in direction (0,0,0) is not possible!");
    	return 0;
    }

    // for speedup:
    double maxrange_sq = maxRange *maxRange;

    // Incremental phase  --------------------------------------------------------- 

    bool done = false;

    while (!done) {
      unsigned int dim;

      // find minimum tMax:
      if (tMax[0] < tMax[1]){
        if (tMax[0] < tMax[2]) dim = 0;
        else                   dim = 2;
      }
      else {
        if (tMax[1] < tMax[2]) dim = 1;
        else                   dim = 2;
      }

      // check for overflow:
      if ((step[dim] < 0 && current_key[dim] == 0)
    		  || (step[dim] > 0 && current_key[dim] == 2* this->tree_max_val-1))
      {
        OCTOMAP_WARNING("Coordinate hit bounds in dim %d, aborting raycast\n", dim);
        // return border point nevertheless:
        end = this->keyToCoord(current_key);
        return 0;
      }

      // advance in direction "dim"
      current_key[dim] += step[dim];
      tMax[dim] += tDelta[dim];


      // generate world coords from key
      end = this->keyToCoord(current_key);

      // check for maxrange:
      if (max_range_set){
        double dist_from_origin_sq(0.0);
        for (unsigned int j = 0; j < 3; j++) {
          dist_from_origin_sq += ((end(j) - origin(j)) * (end(j) - origin(j)));
        }
        if (dist_from_origin_sq > maxrange_sq)
          return 0;
      }
      
      ///  castRay Information Gain !!!!!
      ColorOcTreeNode* currentNode = this->search(current_key);
      if (currentNode){
        if (this->isNodeOccupied(currentNode)) {
	  // verificar si el nodo ya fue tocado
	  return ig_sum;
          
        } else if (this->isNodeUnknown(currentNode)) {
	  //if(currentNode->getColor() == colorTouchedUnknown){
	  // In Kriegel 12 the IG is integrated multiple times, this occur when a several rays pass though a same voxel
	      // el nodo ya fue tocado
	  //}else {
	     if(inBBX(end)){
	       
	      //currentNode->setColor(colorTouchedUnknown);
	      //touchedNodes.push_back(currentNode);
	      double p = currentNode->getOccupancy();
	       
	      ig_sum = ig_sum + (-p * log(p) - (1-p) * log(1-p));
	      //cout << "p " << p << " IG " << ig_sum <<std::endl;
	     }
	  //}
	}
	 // otherwise: node is free and valid, raycasting continues
      } else if (!ignoreUnknownCells){ // no node found, this usually means we are in "unknown" areas
        //OCTOMAP_WARNING_STR("Search failed in OcTree::castRay() => an unknown area was hit in the map: " << end);
        return ig_sum;
      }
    } // end while

    return ig_sum;
}


bool PMVOctreeIGKriegel12::init()
{
  PMVolumetric::init();
  
  std::cout << "---------------- Octree -------------------" << std::endl;
  
  //map = new octomap::ColorOcTree(voxelResolution);
  //map = new COctreeVPL(voxelResolution);
  map = new COctreeKriegel12(voxelResolution);
  
  map->colorOccupied = colorOccupied;
  map->colorUnknown = colorUnmark;
  
  map->colorTouchedOccupied = colorTouchedOccupied;
  map->colorTouchedUnknown = colorTouchedUnkmark;
  
  map->setBBXMax(ObjectBBxMax);
  map->setBBXMin(ObjectBBxMin);
  
  
  // If freeSpace then free voxels are inserted
  if(freeSpace)
    insertFreeSpace(x_free_1,y_free_1,z_free_1,x_free_2,y_free_2,z_free_2);
  
  // The boundries of the object capsule is dilated by voxel resolution
  x_cap_1 -= 2 * voxelResolution;
  y_cap_1 -= 2 * voxelResolution;
  z_cap_1 -= 2 * voxelResolution;
  
  x_cap_2 += 2* voxelResolution;
  y_cap_2 += 2* voxelResolution;
  z_cap_2 += 2* voxelResolution;
  
  std::cout << "-------------------------------------------" << std::endl;
}



PMVOctreeIGKriegel12::PMVOctreeIGKriegel12()
{
 // ok
}

double PMVOctreeIGKriegel12::rayTracingHTMIGKriegel(boost::numeric::ublas::matrix< double > m, EvaluationResult& result)
{
// Funcion revisada! ok!
  double i_ray,j_ray,k_ray;
  
  std::vector< boost::numeric::ublas::matrix<double> >::iterator it;
  BoostMatrix rotated_ray;
  BoostMatrix cero_origin(4,1);
  cero_origin(0,0) = 0;
  cero_origin(1,0) = 0;
  cero_origin(2,0) = 0;
  cero_origin(3,0) = 1;
  BoostMatrix rotated_origin;
  BoostMatrix ray;
  
  octomap::point3d *direction;
  octomap::point3d touched_position;
  
  octomap::ColorOcTreeNode *touched_node;
  octomap::ColorOcTreeNode *origin_node;
  
  //compute origin
  rotated_origin = boost::numeric::ublas::prod(m, cero_origin);
  octomap::point3d origin(rotated_origin(0,0), rotated_origin(1,0), rotated_origin(2,0));
  //cout << "origin: " << std::endl << origin << std::endl;
  
  // check for collision
  origin_node = map->search(origin);
  if(origin_node == NULL){
    //cout << "Origin not found. It could be in a unknown part" << std::endl;
    //cout << origin.x() << " " << origin.y() << " " << origin.z() << std::endl;
    return false;
  }
  
  double ig_sum = 0;
  for(it = rays.begin(); it!= rays.end(); it++){
     /// The ray is rotated and traslated by the rotation matrix
     ray = *it;
     rotated_ray = boost::numeric::ublas::prod(m, ray);
     //cout << "rotated_ray: " << rotated_ray(0,0) << " " <<  rotated_ray(1,0) << " " << rotated_ray(2,0) << std::endl;
     
     //compute direction from position to the rotated ray. This is necesary because the rotated ray was also trasladated by the rotation matrix
     computeRayFromPointToPoint(rotated_origin , rotated_ray, i_ray, j_ray, k_ray);
     
     direction = new octomap::point3d(i_ray, j_ray, k_ray);
     //cout << "direction: " << direction->x() << " " << direction->y() << " " << direction->z() << std::endl;
     
     // if the casted ray returns true a occupied voxel was hit
          
     ig_sum = ig_sum + map->castRayIG(origin, *direction, touched_position);
     
     
     delete direction;
  }
  result.evaluation = ig_sum;
  
//  std::cout << "RT. Occ:" << result.n_occupied << " Occ_sce:" << result.n_occupied_scene 
// 		    << " Unk:" << result.n_unknown <<  " Unk_sce:" << result.n_unknown_scene 
// 		    << " lost:" << result.n_lost << std::endl;

  //map->write("octree_painted.ot");
  map->cleanTouchedVoxels();
  return ig_sum;
}


int PMVOctreeIGKriegel12::evaluateView(ViewStructure& v)
{
  if(!poitsToTheObject(v)){
    //cout << "Sorry no points :(" << std::endl;
    return UNFEASIBLE_VIEW;
  }
  
  EvaluationResult result, countingIG;
  double KriegelIG;
  // first, perform a ray tracing to calculate feasibility
  
  bool valid_result = rayTracingHTM(v.HTM, result);
  v.n_unknown = result.n_unknown;
  v.n_occupied = result.n_occupied;
  v.n_occplane = result.n_unknown;
    
  // Information Gain calculation
  KriegelIG = rayTracingHTMIGKriegel(v.HTM, countingIG);
  
  if(valid_result){
    // See at least an unknowm voxel?
//    if(result.n_unknown == 0 && result.n_unknown_scene ==0){
//      v.eval= 0.0;
//      return UNFEASIBLE_VIEW;
//    } else {	
	// check for registration constraint
	if(registrationConstraint(result)){
	    //view evaluation with frustum information gain
	    v.eval = KriegelIG;
	    return FEASIBLE_VIEW;
	} else {
	   v.eval = 0.0;
	   return UNFEASIBLE_VIEW;
	}
   // }
  } 
  
  return UNFEASIBLE_VIEW;
}

float PMVOctreeIGKriegel12::updateWithScan(std::string file_name_scan, std::string file_name_origin)
{
  float r = PMVOctree::updateWithScan(file_name_scan, file_name_origin);
  map->expand();
  return r;
}
