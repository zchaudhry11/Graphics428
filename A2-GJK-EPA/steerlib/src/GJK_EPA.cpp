#include "..\include\obstacles\GJK_EPA.h"
#include "..\include\obstacles\GJK_EPA.h"
#include "..\include\obstacles\GJK_EPA.h"
#include "..\include\obstacles\GJK_EPA.h"
#include "..\include\obstacles\GJK_EPA.h"
#include "..\include\obstacles\GJK_EPA.h"
#include "..\include\obstacles\GJK_EPA.h"
/*!
*
* \author VaHiD AzIzI
*
*/
static std::vector<Util::Vector> _simplex;

#include "obstacles/GJK_EPA.h"

SteerLib::GJK_EPA::GJK_EPA()
{
}

//Look at the GJK_EPA.h header file for documentation and instructions
bool SteerLib::GJK_EPA::intersect(float& return_penetration_depth, Util::Vector& return_penetration_vector, const std::vector<Util::Vector>& _shapeA, const std::vector<Util::Vector>& _shapeB)
{
	
	bool is_colliding = GJK(_shapeA, _shapeB);
	
	if (is_colliding)
	{
		//Draw simplex


		//EPA here
		return_penetration_vector = EPA(_shapeA, _shapeB, _simplex, return_penetration_depth, return_penetration_vector);
		//std::cout << "\n ShapeA: " << _shapeA[0]; //Check which shape is drawn

		_simplex.clear(); //Clear simplex for next shape
		/*
		for (int i = 0; i < _shapeA.size(); i++)
		{
			std::cout << "\n Shape A: " << _shapeA[i] << "\n";
		}
		for (int i = 0; i < _shapeB.size(); i++)
		{
			std::cout << "\n Shape B: " << _shapeB[i] << "\n";
		}
		*/
	}
	else 
	{
		_simplex.clear(); //Clear simplex for next shape
		return false;
	}	

    return is_colliding; // There is no collision
}

Util::Vector SteerLib::GJK_EPA::EPA(const std::vector<Util::Vector>& _shapeA, const std::vector<Util::Vector>& _shapeB, std::vector<Util::Vector> simplex, float& return_penetration_depth, Util::Vector& return_penetration_vector)
{
	while (true)
	{
		SteerLib::GJK_EPA::Edge closestEdge = SteerLib::GJK_EPA::getClosestEdge(_simplex);

		Util::Vector edgePoint = SteerLib::GJK_EPA::GJK_Support(_shapeA, _shapeB, closestEdge.normal); //Get point towards the edge

		float distance = Util::dot(edgePoint, closestEdge.normal);
		if (distance - closestEdge.distance < 0.0000000001)
		{
			return_penetration_depth = distance;
			return closestEdge.normal;
		}
		else
		{
			_simplex[closestEdge.nVertIndex] = edgePoint;
		}
	}


	return return_penetration_vector;
}

SteerLib::GJK_EPA::Edge SteerLib::GJK_EPA::getClosestEdge(std::vector<Util::Vector> simplex)
{
	SteerLib::GJK_EPA::Edge closestEdge = SteerLib::GJK_EPA::Edge(); //Edge closest to origin on MS
	closestEdge.distance = 999999;

	for (int i = 0; i < simplex.size() - 1; i++)
	{
		int next = i + 1; //Next point in simplex

		if (next == simplex.size() - 1)
		{
			next = 0;
		}

		Util::Vector pointA = simplex[i];
		Util::Vector pointB = simplex[next];

		Util::Vector edge = pointB.operator-(pointA);
		Util::Vector nEdge = edge.operator-();
		//-A(B*C) + B(A*C)
		Util::Vector normal = nEdge.operator*(Util::dot(pointA, edge)) + pointA.operator*(Util::dot(edge, edge));
		normal = Util::normalize(normal);

		float distance = Util::dot(normal, pointB);

		if (distance < closestEdge.distance)
		{
			closestEdge.distance = distance;
			closestEdge.normal = normal;
			closestEdge.nVertIndex = next;
		}
	}

	return closestEdge;
}

bool SteerLib::GJK_EPA::GJK(const std::vector<Util::Vector>& _shapeA, const std::vector<Util::Vector>& _shapeB)
{
	Util::Vector direction = Util::Vector(1, 0, -1); //Set arbitrary direction

	_simplex.push_back(GJK_Support(_shapeA, _shapeB, direction)); //Add first vertex in simplex

	direction = direction.operator-();

	while (true)
	{
		_simplex.push_back(GJK_Support(_shapeA, _shapeB, direction)); //Add next point to simplex

		//Check to see if origin was passed
		float distance = Util::dot(_simplex[_simplex.size() - 1], direction);
		if (distance <= 0)
		{
			//std::cout << "\n entered";
			return false; //Last point added to simplex did not pass the origin
		}
		else if (GJK_originCheck(direction)) //Check if origin exists inside the simplex otherwise change direction
		{
			//std::cout << "\n orig check: ";
			return true;
		}
	}

	return false;
}

Util::Vector SteerLib::GJK_EPA::GJK_Support(const std::vector<Util::Vector>& _shapeA, const std::vector<Util::Vector>& _shapeB, Util::Vector direction)
{
	Util::Vector pointA = getFarthestPoint(_shapeA, direction);
	Util::Vector pointB = getFarthestPoint(_shapeB, direction.operator-());

	Util::Vector pointC = pointA.operator-(pointB); //Point on edge of MS

	return pointC;
}

bool SteerLib::GJK_EPA::GJK_originCheck(Util::Vector & direction)
{
	//std::cout << "\n numVerts: " << _simplex.size();
	Util::Vector pointA = _simplex[_simplex.size() - 1]; //Get latest vertex from simplex

	Util::Vector AO = pointA.operator-(); //Origin - Point A

	if (_simplex.size() == 3) //Entire triangle exists with verts A, B, C
	{
		Util::Vector pointB = _simplex[1];
		Util::Vector pointC = _simplex[0];

		Util::Vector AB = pointB.operator-(pointA);
		Util::Vector AC = pointC.operator-(pointA);

		//Simplex check on pointC
		//Get direction perpendicular to AB
		Util::Vector newDirection = Util::Vector(-AB.z, 0, AB.x);
		direction = newDirection; 

		//Check to see if new direction will move towards origin
		float distance = Util::dot(direction, pointC);
		if (distance > 0)
		{
			direction = direction.operator-();
		}

		//std::cout << "\n 3dir: " << direction;

		//If new direction is moving towards origin, pointC is far from the origin and should be replaced
		distance = Util::dot(direction, AO);
		if (distance > 0)
		{
			//std::cout << "\n removed C";
			_simplex.erase(_simplex.begin());
			return false;
		}

		//Simplex check on pointB
		//Get direction perpendicular to AC
		newDirection = Util::Vector(-AC.z, 0, AC.x);
		direction = newDirection;

		//Check to see if new direction moves towards origin
		distance = Util::dot(direction, pointB);
		if (distance > 0)
		{
			direction = direction.operator-();
		}

		//If new direction is moving towards origin, pointB is far from the origin and should be replaced
		distance = Util::dot(direction, AO);
		if (distance > 0)
		{
			//std::cout << "\n removed B";
			_simplex.erase(_simplex.begin() + 1);
			return false;
		}

		//Simplex points pass the checks so origin must be inside the current triangle
		return true;
	}
	else //Simplex is not a triangle
	{
		//Get direction perpendicular to AB and origin
		Util::Vector pointB = _simplex[0];

		Util::Vector AB = pointB.operator-(pointA);

		Util::Vector newDirection = Util::Vector(-AB.z, 0, AB.x);
		direction = newDirection;

		float distance = Util::dot(direction, AO);
		if (distance < 0)
		{
			direction = direction.operator-();
		}
	}

	return false;
}

Util::Vector SteerLib::GJK_EPA::getFarthestPoint(const std::vector<Util::Vector>& _shape, Util::Vector direction)
{
	float distance = Util::dot(_shape[0], direction);
	int vertex = 0;

	//Loop through all vertices in a shape and find which one is the farthest away from our direction
	for (int i = 0; i < _shape.size(); i++)
	{
		float newDist = Util::dot(_shape[i], direction);
		if (newDist > distance)
		{
			distance = newDist;
			vertex = i;
		}
	}

	return _shape[vertex];
}
