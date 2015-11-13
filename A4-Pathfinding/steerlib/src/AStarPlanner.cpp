//
// Copyright (c) 2009-2015 Glen Berseth, Mubbasir Kapadia, Shawn Singh, Petros Faloutsos, Glenn Reinman, Rahul Shome
// See license.txt for complete license.
//


#include <vector>
#include <stack>
#include <set>
#include <map>
#include <iostream>
#include <algorithm> 
#include <functional>
#include <queue>
#include <math.h>
#include "planning/AStarPlanner.h"


#define COLLISION_COST  1000
#define GRID_STEP  1
#define OBSTACLE_CLEARANCE 1
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define DIAG_COST 2

namespace SteerLib
{
	AStarPlanner::AStarPlanner(){}

	AStarPlanner::~AStarPlanner(){}

	bool AStarPlanner::canBeTraversed ( int id ) 
	{
		double traversal_cost = 0;
		int current_id = id;
		unsigned int x,z;
		gSpatialDatabase->getGridCoordinatesFromIndex(current_id, x, z);
		int x_range_min, x_range_max, z_range_min, z_range_max;

		x_range_min = MAX(x-OBSTACLE_CLEARANCE, 0);
		x_range_max = MIN(x+OBSTACLE_CLEARANCE, gSpatialDatabase->getNumCellsX());

		z_range_min = MAX(z-OBSTACLE_CLEARANCE, 0);
		z_range_max = MIN(z+OBSTACLE_CLEARANCE, gSpatialDatabase->getNumCellsZ());


		for (int i = x_range_min; i<=x_range_max; i+=GRID_STEP)
		{
			for (int j = z_range_min; j<=z_range_max; j+=GRID_STEP)
			{
				int index = gSpatialDatabase->getCellIndexFromGridCoords( i, j );
				traversal_cost += gSpatialDatabase->getTraversalCost ( index );
				
			}
		}

		if ( traversal_cost > COLLISION_COST ) 
			return false;
		return true;
	}



	Util::Point AStarPlanner::getPointFromGridIndex(int id)
	{
		Util::Point p;
		gSpatialDatabase->getLocationFromIndex(id, p);
		return p;
	}



	bool AStarPlanner::computePath(std::vector<Util::Point>& agent_path,  Util::Point start, Util::Point goal, SteerLib::GridDatabase2D * _gSpatialDatabase, bool append_to_path)
	{
		gSpatialDatabase = _gSpatialDatabase;
		
		std::deque<Node> openNodes; //List of nodes to visit and their priority
		std::vector<Node> visitedNodes; //List of nodes that have already been visited
		Node currNode;
		std::vector<Node> neighbors;
		std::map<int, int> costSoFar; //Id, priority

		//Add start node to queue
		Node startNode;

		startNode.position = start;
		startNode.gCost = 0; //should be 0
		startNode.hCost = computeHCost(0, startNode, goal);

		std::cout << "\n Calculating start hCost: " << startNode.hCost;

		startNode.fCost = startNode.gCost + startNode.fCost;
		startNode.isDiag = false;
		startNode.id = 0;

		openNodes.push_back(startNode);
		agent_path.push_back(start);
		costSoFar[0] = 0;

		//Loop until there are no more nodes to visit
		while (openNodes.size() > 0)
		{
			//Select node with lowest fCost
			int lowest = 9999;
			int index = 0;
			for (int b = 0; b < openNodes.size(); b++)
			{
				//std::cout << "\n POTENTIAL NODE: " << openNodes[b].id << " fCost: " << openNodes[b].fCost << " hCost: " << openNodes[b].hCost << " gCost: " << openNodes[b].gCost;
				if (openNodes[b].fCost < lowest) //For later part, add equality check and compare g/h costs
				{
					lowest = openNodes[b].fCost;
					currNode = openNodes[b];
					index = b;
				}
			}

			openNodes.erase(openNodes.begin() + index);
			std::cout << "\n OPEN SIZE: " << openNodes.size();
			std::cout << "\n currNodeID: " << currNode.id << " currNodePOS: " << currNode.position;

			visitedNodes.push_back(currNode);

			//Found goal
			if (currNode.position.operator==(goal))
			{
				std::cout << "\n GOAL REACHED!";
				break;
			}

			//Get the neighbors of the current node
			unsigned int currIndex = gSpatialDatabase->getCellIndexFromLocation(currNode.position);
			unsigned int currXCoord = 0;
			unsigned int currZCoord = 0;
			gSpatialDatabase->getGridCoordinatesFromIndex(currIndex, currXCoord, currZCoord);
			
			unsigned int neighborXCoord = currXCoord;
			unsigned int neighborZCoord = currZCoord;
			float travCost = 0;

			neighborXCoord--; //Check cell at (x-1, z)
			travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));

			//Check if neighboring cell is a node. Obstacles have a traversal cost of 1000
			if (travCost >= 0 && travCost < COLLISION_COST)
			{
				//Add neighbor to list
				Node neighborNode;
				Util::Point neighborPos;
				gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos);
				neighborNode.position = neighborPos;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = false;
				neighborNode.id = currNode.id + 1;
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);

				//Reset values for the next check
				neighborXCoord += 2; //Check cell at (x+1, z)
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}
			else //Not a traversible node
			{
				//Reset values for the next check
				neighborXCoord += 2; //Check cell at (x+1, z)
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}

			if (travCost >= 0 && travCost < COLLISION_COST)
			{
				//Add neighbor to list
				Node neighborNode;
				Util::Point neighborPos;
				gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos);
				neighborNode.position = neighborPos;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = false;
				neighborNode.id = currNode.id + 2;
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);

				//Reset values for the next check
				neighborXCoord = currXCoord; //Check cell at (x, z-1)
				neighborZCoord--;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}
			else //Not a traversible node
			{
				//Reset values for the next check
				neighborXCoord = currXCoord; //Check cell at (x, z-1)
				neighborZCoord--;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}

			if (travCost >= 0 && travCost < COLLISION_COST)
			{
				//Add neighbor to list
				Node neighborNode;
				Util::Point neighborPos;
				gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos);
				neighborNode.position = neighborPos;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = false;
				neighborNode.id = currNode.id + 3;
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);

				if (neighborNode.id == 3)
				{
					std::cout << " \n travCost: " << travCost << " gcost: " << neighborNode.gCost << " hCost: " << neighborNode.hCost << " ID: " << neighborNode.id << " POS: " << neighborNode.position;
				}

				//Reset values for the next check
				neighborZCoord += 2; //Check cell at (x, z+1)
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}
			else //Not a traversible node
			{
				//Reset values for the next check
				neighborZCoord += 2; //Check cell at (x, z+1)
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}

			if (travCost >= 0 && travCost < COLLISION_COST)
			{
				//Add neighbor to list
				Node neighborNode;
				Util::Point neighborPos;
				gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos);
				neighborNode.position = neighborPos;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = false;
				neighborNode.id = currNode.id + 4;
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);

				//Reset values for the next check
				neighborZCoord += 2; //Check cell at (x, z+1)
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}

			//If diagonals are enabled, then also check points at (x-1, z-1), (x-1, z+1), (x+1, z-1), and (x+1, z+1)

			
			if (visitedNodes.size() > 2000)
			{
				break;
			}
			
			//Loop through neighbors and see if they are usable nodes
			for (int i = 0; i < neighbors.size(); i++)
			{
				std::cout << "\n visited: " << visitedNodes.size();
				//std::cout << "\n v: " << visitedNodes[0].position;
				//std::cout << "\n v5: " << visitedNodes[1].position;
				//std::cout << "\n open: " << openNodes.size();
				//std::cout << "\n nPos: " << neighbors[i].position;
				//std::cout << "\n neighbors: " << neighbors.size();

				bool nodeVisited = false;

				for (int q = 0; q < visitedNodes.size(); q++)
				{
					if (visitedNodes[q].position.operator==(neighbors[i].position))
					{
						nodeVisited = true;
					}
				}

				//Check if neighbor was previously visited
				if (nodeVisited)
				{
					std::cout << "\n VISITED OLD NODE!";
					/*
					for (int b = 0; b < openNodes.size(); b++)
					{
						openNodes.pop_front();
					}
					*/
					
					continue;
				}
				else //Neighbor was not visited before
				{
					int newCost = 0;

					if (neighbors[i].isDiag == false)
					{
						//newCost = currNode.gCost + 1;

						newCost = costSoFar[currNode.id] + 1;
					}
					else
					{
						newCost = currNode.gCost + DIAG_COST;
					}	
					newCost = costSoFar[currNode.id] + 1;
					std::cout << "\n newCost: " << newCost << " nCost: " << costSoFar[neighbors[i].id] << " ID: " << neighbors[i].id;
					if (newCost < costSoFar[neighbors[i].id])
					{
						//Add node to agent_path
						std::cout << "\n NEW COST!";
						neighbors[i].gCost = newCost;
						neighbors[i].hCost = computeHCost(0, neighbors[i], goal);
						neighbors[i].fCost = neighbors[i].gCost + neighbors[i].hCost;
						openNodes.push_back(neighbors[i]);
						agent_path.push_back(neighbors[i].position);
					}
					else if (currNode.id == 0)
					{
						std::cout << "\n IS START NODE!";
						neighbors[i].gCost = newCost;
						neighbors[i].hCost = computeHCost(0, neighbors[i], goal);
						neighbors[i].fCost = neighbors[i].gCost + neighbors[i].hCost;
						openNodes.push_back(neighbors[i]);
						std::cout << "\n ID: " << neighbors[i].id << " POS: " << neighbors[i].position;
						agent_path.push_back(neighbors[i].position);
					}
					else
					{
						//Node wasnt selected
						std::cout << "\n REJECTED: " << neighbors[i].id << " gCost: " << neighbors[i].gCost << " hCost: " << neighbors[i].hCost;

						openNodes.push_back(neighbors[i]);
						//visitedNodes.push_back(neighbors[i]);
					}
				}
			}

			neighbors.clear(); //Clear list of neighbors for the next node
			// Remove current node from open list and add it to closed list

		}

		std::cout << "\n FINISHED!";

		//Clean duplicate entries out of path


		//agent_path.push_back(goal);
		return true;
	}

	int AStarPlanner::computeHCost(int heuristic, Node start, Util::Point target)
	{
		//Obtain coordinates for start and target
		int startIndex = gSpatialDatabase->getCellIndexFromLocation(start.position);
		int targetIndex = gSpatialDatabase->getCellIndexFromLocation(target);

		unsigned int startX, startZ, targetX, targetZ;

		int startXi, startZi, targetXi, targetZi;

		gSpatialDatabase->getGridCoordinatesFromIndex(startIndex, startX, startZ);
		gSpatialDatabase->getGridCoordinatesFromIndex(targetIndex, targetX, targetZ);

		//Find Manhatten Distance
		if (heuristic == 0)
		{
			startXi = startX; startZi = startZ; targetXi = targetX; targetZi = targetZ;

			int finalH = (abs(startXi - targetXi) + abs(startZi - targetZi));

			return finalH;
		}
		else if (heuristic == 1) //Find Euclidean Distance
		{
			return (int)sqrtf(pow((startX - targetX), 2) + pow((startZ - targetZ), 2));
		}

		return 0;
	}

	

}