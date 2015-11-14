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
		std::map<int, int> parents; //ID of node, value = parent of that node. -1 means no parent
		//Add start node to queue
		Node startNode;

		Util::Point startPos;
		unsigned int startX, startZ, startIndex;
		startIndex = gSpatialDatabase->getCellIndexFromLocation(start);
		gSpatialDatabase->getGridCoordinatesFromIndex(startIndex, startX, startZ);
		gSpatialDatabase->getLocationFromIndex(startIndex, startPos);

		startNode.position = startPos;
		startNode.gCost = 0; //should be 0
		startNode.hCost = computeHCost(0, startNode, goal);

		std::cout << "\n Calculating start hCost: " << startNode.hCost;

		startNode.fCost = startNode.gCost + startNode.fCost;
		startNode.isDiag = false;
		startNode.id = gSpatialDatabase->getCellIndexFromLocation(startNode.position);

		openNodes.push_back(startNode);
		agent_path.push_back(startNode.position);
		costSoFar[0] = 0;
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
		std::cout << "\n GOAL: " << goalPos;

		//Loop until there are no more nodes to visit
		while (openNodes.size() > 0)
		{
			/*
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
			*/
			currNode = openNodes.front();
			currNode.id = gSpatialDatabase->getCellIndexFromLocation(currNode.position);
			visitedNodes.push_back(openNodes.front());
			openNodes.pop_front();
			//openNodes.erase(openNodes.begin() + index);
			//std::cout << "\n OPEN SIZE: " << openNodes.size();
			//std::cout << "\n visited: " << visitedNodes.size();
			//std::cout << "\n currNodeID: " << currNode.id << " currNodePOS: " << currNode.position;

			//Found goal
			if (currNode.position.operator==(goalPos))
			{
				std::cout << "\n GOAL REACHED!";
				//parents[goalNode.id] = visitedNodes[visitedNodes.size()-2].id; //Set parent node
				//return false;
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
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = false;
				//neighborNode.id = currNode.id + 1;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);
				//std::cout << " \n travCost: " << travCost << " ID: " << neighborNode.id;

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
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = false;
				//neighborNode.id = currNode.id + 2;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);
				//std::cout << " \n travCost: " << travCost << " ID: " << neighborNode.id;

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
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = false;
				//neighborNode.id = currNode.id + 3;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);
				//std::cout << " \n travCost: " << travCost << " ID: " << neighborNode.id;

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
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = false;
				//neighborNode.id = currNode.id + 4;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);
				//std::cout << " \n travCost: " << travCost << " ID: " << neighborNode.id;

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
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = true;
				//neighborNode.id = currNode.id + 5;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);
				//std::cout << " \n travCost: " << travCost << " ID: " << neighborNode.id;

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
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = true;
				//neighborNode.id = currNode.id + 6;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);
				//std::cout << " \n travCost: " << travCost << " ID: " << neighborNode.id;

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
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = true;
				//neighborNode.id = currNode.id + 7;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);
				//std::cout << " \n travCost: " << travCost << " ID: " << neighborNode.id;

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
				neighborNode.gCost = travCost + currNode.gCost;
				neighborNode.hCost = computeHCost(0, neighborNode, goal);
				neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;
				neighborNode.isDiag = true;
				//neighborNode.id = currNode.id + 8;
				neighborNode.id = gSpatialDatabase->getCellIndexFromLocation(neighborNode.position);
				costSoFar[neighborNode.id] = neighborNode.gCost;
				neighbors.push_back(neighborNode);
				//std::cout << " \n travCost: " << travCost << " ID: " << neighborNode.id;
			}

			//Loop through neighbors and see if they are usable nodes
			for (int i = 0; i < neighbors.size(); i++)
			{
				//std::cout << "\n visited: " << visitedNodes.size();
				//std::cout << "\n open: " << openNodes.size();
				//std::cout << "\n nPos: " << neighbors[i].position;
				//std::cout << "\n neighbors: " << neighbors.size();

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
					//std::cout << "\n VISITED OLD NODE!";
					
					continue;
				}
				else //Neighbor was not visited before
				{
					//Add node to agent_path
					//std::cout << "\n ADDED NODE!";
					//neighbors[i].gCost = newCost;
					neighbors[i].hCost = computeHCost(0, neighbors[i], goal);
					neighbors[i].fCost = neighbors[i].gCost + neighbors[i].hCost;
					parents[neighbors[i].id] = currNode.id; //Set parent node
					openNodes.push_back(neighbors[i]);
					//agent_path.push_back(neighbors[i].position);
					visitedNodes.push_back(neighbors[i]);
				}
			}
			neighbors.clear(); //Clear list of neighbors for the next node
		}

		//std::cout << "\n FINISHED!";

		int pathIndex = parents[goalNode.id];
		std::cout << "\n pI: " << pathIndex << " GOAL: " << gSpatialDatabase->getCellIndexFromLocation(goalNode.position);
		std::vector<Util::Point> finalPath;
		finalPath.push_back(goalNode.position);
		int numNodes = 0;
		while (pathIndex != startNode.id) //17917
		{
			for (int i = 0; i < visitedNodes.size()-1; i++)
			{
				if (visitedNodes[i].id == pathIndex && pathIndex != -1)
				{
					numNodes++;
					finalPath.push_back(visitedNodes[i].position);
					pathIndex = parents[visitedNodes[i].id];
					//std::cout << "\n visited ID: " << visitedNodes[i].id;
					break;
				}
			}
			//std::cout << "\n path: " << pathIndex;
		}

		for (int i = 0; i < visitedNodes.size() - 1; i++)
		{
			//std::cout << "\n vnID: " << visitedNodes[i].id;
		}

		std::map<int, int>::iterator it;

		for (it = parents.begin(); it != parents.end(); it++)
		{
			//std::cout << "\n nodeID: " << it->first << " parentNode: " << it->second;
		}

		finalPath.push_back(startNode.position);

		for (int i = 0; i < visitedNodes.size(); i++)
		{
			if (visitedNodes[i].id == parents[goalNode.id])
			{
				std::cout << "\n GOAL POS: " << goalNode.position << " PARENT: " << visitedNodes[i].position << " VISITED: " << visitedNodes.size() << " PATH SIZE: " << finalPath.size();
			}
		}

		while (numNodes > 0)
		{
			//std::cout << "\n fP: " << finalPath[i];
			if (numNodes - 1 >= 0)
			{
				agent_path.push_back(finalPath[numNodes - 1]);
				std::cout << "\n PATH POS: " << finalPath[numNodes - 1];
			}
			
			numNodes--;
		}

		for (int i = 0; i < visitedNodes.size(); i++)
		{
			if (visitedNodes[i].position.operator==(agent_path[0]))
			{
				std::cout << "\n id: " << visitedNodes[i].id << " pos: " << visitedNodes[i].position;
			}
			if (visitedNodes[i].position.operator==(agent_path[1]))
			{
				std::cout << "\n id: " << visitedNodes[i].id << " pos: " << visitedNodes[i].position;
			}
		}

		std::cout << "\n";

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