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


#define COLLISION_COST  1000    //ingress requires 9200
#define GRID_STEP  1
#define OBSTACLE_CLEARANCE 1
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define DIAG_COST 1
#define HEURISTIC 0
#define HEURISTIC_WEIGHT 1
#define TIEBREAKER -1

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
		std::map<int, int> parents; //ID of node, value = parent of that node. -1 means no parent
		//Add start node to queue
		Node startNode;

		Util::Point startPos;
		unsigned int startX, startZ, startIndex;
		startIndex = gSpatialDatabase->getCellIndexFromLocation(start);
		gSpatialDatabase->getGridCoordinatesFromIndex(startIndex, startX, startZ);
		gSpatialDatabase->getLocationFromIndex(startIndex, startPos);

		startNode.position = startPos;
		startNode.gCost = 0;
		startNode.hCost = computeHCost(HEURISTIC, startNode, goal);

		startNode.fCost = startNode.gCost + startNode.fCost;
		startNode.isDiag = false;
		startNode.id = gSpatialDatabase->getCellIndexFromLocation(startNode.position);

		openNodes.push_back(startNode);
		agent_path.push_back(startNode.position);
		parents[startNode.id] = -1; //start node has no parent

		//Set goal node
		Node goalNode;
		Util::Point goalPos;
		unsigned int goalX, goalZ, goalIndex;
		goalIndex = gSpatialDatabase->getCellIndexFromLocation(goal);
		gSpatialDatabase->getGridCoordinatesFromIndex(goalIndex, goalX, goalZ);
		gSpatialDatabase->getLocationFromIndex(goalIndex, goalPos);
		goalNode.id = goalIndex;
		goalNode.position = goalPos;

		//Loop until there are no more nodes to visit
		while (openNodes.size() > 0)
		{
			//Select node with lowest fCost
			int lowest = 9999;
			int index = 0;
			bool breakTie = false;
			std::vector<Node> equalFNodes;
			for (int b = 0; b < openNodes.size(); b++)
			{
				if (openNodes[b].fCost < lowest) //For later part, add equality check and compare g/h costs
				{
					lowest = openNodes[b].fCost;
					currNode = openNodes[b];
					index = b;
				}
				else if (openNodes[b].fCost == lowest && TIEBREAKER != -1)
				{
					breakTie = true;
					equalFNodes.push_back(openNodes[b]);
				}
			}
			
			if (breakTie == false)
			{
				currNode.id = gSpatialDatabase->getCellIndexFromLocation(currNode.position);
				visitedNodes.push_back(openNodes[index]);
				openNodes.erase(openNodes.begin() + index);
			}
			else
			{
				int lowestG = 9999;
				int lowestIndex = 0;
				int highestG = -1;
				int highestIndex = 0;
				for (int i = 0; i < equalFNodes.size()-1; i++)
				{
					if (equalFNodes[i].gCost < lowestG)
					{
						lowestG = equalFNodes[i].gCost;
						lowestIndex = i;
					}
					if (equalFNodes[i].gCost > highestG)
					{
						highestG = equalFNodes[i].gCost;
						highestIndex = i;
					}
				}

				if (TIEBREAKER == 0) //Select lowest gCost
				{
					currNode = equalFNodes[lowestIndex];
					currNode.id = gSpatialDatabase->getCellIndexFromLocation(currNode.position);
					visitedNodes.push_back(openNodes[lowestIndex]);
					openNodes.erase(openNodes.begin() + lowestIndex);
				}
				else if (TIEBREAKER > 0) //Select highest gCost
				{
					currNode = equalFNodes[highestIndex];
					currNode.id = gSpatialDatabase->getCellIndexFromLocation(currNode.position);
					visitedNodes.push_back(openNodes[highestIndex]);
					openNodes.erase(openNodes.begin() + highestIndex);
				}
			}

			//Found goal
			if (currNode.position.operator==(goalPos))
			{
				//std::cout << "\n GOAL REACHED!";
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

			Node neighborNode;
			Util::Point neighborPos;

			neighborXCoord--; //Check cell at (x-1, z)
			travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));

			//Check if neighboring cell is a node. Obstacles have a traversal cost of 1000
			gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos); //(x-1, z)
			if (travCost >= 0 && travCost < COLLISION_COST && canBeTraversed(gSpatialDatabase->getCellIndexFromLocation(neighborPos))) 
			{
				//Add neighbor to list				
				neighborNode.position = neighborPos;
				neighborNode.isDiag = false;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(HEURISTIC, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				
				//neighborNode.id = currNode.id + 1;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				neighbors.push_back(neighborNode);

				//Reset values for the next check
				neighborXCoord = currXCoord; //Check cell at (x+1, z)
				neighborZCoord = currZCoord;
				neighborXCoord += 1; 
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}
			else //Not a traversible node
			{
				//Reset values for the next check
				neighborXCoord = currXCoord; //Check cell at (x+1, z)
				neighborZCoord = currZCoord;
				neighborXCoord += 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}

			gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos); //(x+1, z)
			if (travCost >= 0 && travCost < COLLISION_COST && canBeTraversed(gSpatialDatabase->getCellIndexFromLocation(neighborPos))) 
			{
				//Add neighbor to list
				neighborNode.position = neighborPos;
				neighborNode.isDiag = false;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(HEURISTIC, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				
				//neighborNode.id = currNode.id + 2;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				neighbors.push_back(neighborNode);

				//Reset values for the next check
				neighborXCoord = currXCoord; //Check cell at (x, z-1)
				neighborZCoord = currZCoord;
				neighborZCoord -= 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}
			else //Not a traversible node
			{
				//Reset values for the next check
				neighborXCoord = currXCoord;//Check cell at (x, z-1)
				neighborZCoord = currZCoord;
				neighborZCoord -= 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}

			gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos); //(x, z-1)
			if (travCost >= 0 && travCost < COLLISION_COST && canBeTraversed(gSpatialDatabase->getCellIndexFromLocation(neighborPos)))
			{
				//Add neighbor to list
				neighborNode.position = neighborPos;
				neighborNode.isDiag = false;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(HEURISTIC, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				
				//neighborNode.id = currNode.id + 3;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				neighbors.push_back(neighborNode);

				//Reset values for the next check
				neighborXCoord = currXCoord; //Check cell at (x, z+1)
				neighborZCoord = currZCoord;
				neighborZCoord += 1; 
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}
			else //Not a traversible node
			{
				//Reset values for the next check
				neighborXCoord = currXCoord; //Check cell at (x, z+1)
				neighborZCoord = currZCoord;
				neighborZCoord += 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}

			gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos); //(x, z+1)
			if (travCost >= 0 && travCost < COLLISION_COST && canBeTraversed(gSpatialDatabase->getCellIndexFromLocation(neighborPos))) 
			{
				//Add neighbor to list
				neighborNode.position = neighborPos;
				neighborNode.isDiag = false;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(HEURISTIC, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				
				//neighborNode.id = currNode.id + 4;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				neighbors.push_back(neighborNode);

				//Reset values for the next check
				neighborXCoord = currXCoord; //Check cell at (x-1, z-1)
				neighborZCoord = currZCoord;
				neighborXCoord -= 1;
				neighborZCoord -= 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}
			else //Not a traversible node
			{
				//Reset values for the next check
				neighborXCoord = currXCoord; //Check cell at (x-1, z-1)
				neighborZCoord = currZCoord;
				neighborXCoord -= 1;
				neighborZCoord -= 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}

			gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos); //(x-1, z-1)
			if (travCost >= 0 && travCost < COLLISION_COST && canBeTraversed(gSpatialDatabase->getCellIndexFromLocation(neighborPos))) 
			{
				//Add neighbor to list
				neighborNode.position = neighborPos;
				neighborNode.isDiag = true;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(HEURISTIC, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				
				//neighborNode.id = currNode.id + 5;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				neighbors.push_back(neighborNode);

				//Reset values for the next check
				neighborXCoord = currXCoord;
				neighborZCoord = currZCoord;
				neighborZCoord += 1; //Check cell at (x-1, z+1)
				neighborXCoord -= 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}
			else //Not a traversible node
			{
				//Reset values for the next check
				neighborXCoord = currXCoord;
				neighborZCoord = currZCoord;
				neighborZCoord += 1; //Check cell at (x-1, z+1)
				neighborXCoord -= 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}
			
			gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos); //(x-1, z+1)
			if (travCost >= 0 && travCost < COLLISION_COST && canBeTraversed(gSpatialDatabase->getCellIndexFromLocation(neighborPos)))
			{
				//Add neighbor to list
				neighborNode.position = neighborPos;
				neighborNode.isDiag = true;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(HEURISTIC, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				
				//neighborNode.id = currNode.id + 6;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				neighbors.push_back(neighborNode);

				//Reset values for the next check
				neighborXCoord = currXCoord;
				neighborZCoord = currZCoord;
				neighborZCoord -= 1; //Check cell at (x+1, z-1)
				neighborXCoord += 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}
			else //Not a traversible node
			{
				//Reset values for the next check
				neighborXCoord = currXCoord;
				neighborZCoord = currZCoord;
				neighborZCoord -= 1; //Check cell at (x+1, z-1)
				neighborXCoord += 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}

			gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos); //(x+1, z-1)
			if (travCost >= 0 && travCost < COLLISION_COST && canBeTraversed(gSpatialDatabase->getCellIndexFromLocation(neighborPos)))
			{
				//Add neighbor to list
				neighborNode.position = neighborPos;
				neighborNode.isDiag = true;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(HEURISTIC, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				
				//neighborNode.id = currNode.id + 7;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				neighbors.push_back(neighborNode);

				//Reset values for the next check
				neighborXCoord = currXCoord;
				neighborZCoord = currZCoord;
				neighborZCoord += 1; //Check cell at (x+1, z+1)
				neighborXCoord += 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}
			else //Not a traversible node
			{
				//Reset values for the next check
				neighborXCoord = currXCoord;
				neighborZCoord = currZCoord;
				neighborZCoord += 1; //Check cell at (x+1, z+1)
				neighborXCoord += 1;
				travCost = gSpatialDatabase->getTraversalCost(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord));
			}

			gSpatialDatabase->getLocationFromIndex(gSpatialDatabase->getCellIndexFromGridCoords(neighborXCoord, neighborZCoord), neighborPos); //(x+1, z+1)
			if (travCost >= 0 && travCost < COLLISION_COST && canBeTraversed(gSpatialDatabase->getCellIndexFromLocation(neighborPos)))
			{
				//Add neighbor to list
				neighborNode.position = neighborPos;
				neighborNode.isDiag = true;
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(HEURISTIC, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				
				//neighborNode.id = currNode.id + 8;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				neighbors.push_back(neighborNode);
			}

			//Loop through neighbors and see if they are usable nodes
			for (int i = 0; i < neighbors.size(); i++)
			{
				int newCost = 0;

				if (neighbors[i].isDiag == false)
				{
					newCost = currNode.gCost + 1;
				}
				else 
				{
					newCost = currNode.gCost + DIAG_COST;
				}

				bool nodeVisited = false;
				for (int q = 0; q < visitedNodes.size()-1; q++)
				{
					if (visitedNodes[q].position.operator==(neighbors[i].position))
					{
						nodeVisited = true;
					}
				}

				//Check if neighbor was previously visited
				if (nodeVisited == true)
				{					
					continue;
				}
				else //Neighbor was not visited before
				{
					if (newCost <= neighbors[i].gCost)
					{
						//Add node to agent_path
						neighbors[i].gCost = newCost;
						neighbors[i].hCost = computeHCost(HEURISTIC, neighbors[i], goal);
						neighbors[i].fCost = neighbors[i].gCost + neighbors[i].hCost;
						parents[neighbors[i].id] = currNode.id; //Set parent node
						openNodes.push_back(neighbors[i]);
						visitedNodes.push_back(neighbors[i]);
					}
				}
			}
			neighbors.clear(); //Clear list of neighbors for the next node
		}

		int pathIndex = parents[goalNode.id];

		std::vector<Util::Point> finalPath;
		finalPath.push_back(goalNode.position);

		int numNodes = 0;
		while (pathIndex != startNode.id || pathIndex != 0)
		{
			for (int i = 0; i < visitedNodes.size()-1; i++)
			{
				if (visitedNodes[i].id == pathIndex && pathIndex != -1)
				{
					numNodes++;
					finalPath.push_back(visitedNodes[i].position);
					pathIndex = parents[visitedNodes[i].id];
					break;
				}
			}
			//std::cout << "\n path: " << pathIndex;

			if (pathIndex == -1 || pathIndex == 0)
			{
				break;
			}
		}

		finalPath.push_back(startNode.position);

		while (numNodes > 0)
		{
			if (numNodes - 1 >= 0)
			{
				agent_path.push_back(finalPath[numNodes - 1]);
			}
			numNodes--;
		}
		std::cout << "\nNumber of expanded nodes: " << visitedNodes.size();
		std::cout << "\nLength of solution path: " << agent_path.size() << "\n";
		return true;
	}

	int AStarPlanner::computeHCost(int heuristic, Node start, Util::Point target)
	{
		//Obtain coordinates for start and target
		int startIndex = gSpatialDatabase->getCellIndexFromLocation(start.position);
		int targetIndex = gSpatialDatabase->getCellIndexFromLocation(target);

		unsigned int startX, startZ, targetX, targetZ;

		int startXi, startZi, targetXi, targetZi;

		float startXf, startZf, targetXf, targetZf;

		gSpatialDatabase->getGridCoordinatesFromIndex(startIndex, startX, startZ);
		gSpatialDatabase->getGridCoordinatesFromIndex(targetIndex, targetX, targetZ);

		//Find Manhatten Distance
		if (heuristic == 0)
		{
			startXi = startX; startZi = startZ; targetXi = targetX; targetZi = targetZ;

			int finalH = (abs(startXi - targetXi) + abs(startZi - targetZi));

			return finalH * HEURISTIC_WEIGHT;
		}
		else if (heuristic == 1) //Find Euclidean Distance
		{
			
			startXf = startX; startZf = startZ; targetXf = targetX; targetZf = targetZ;

			int finalH = sqrt(pow((startXf - targetXf), 2) + pow((startZf - targetZf), 2));

			return finalH * HEURISTIC_WEIGHT;
		}

		return 0;
	}

	

}