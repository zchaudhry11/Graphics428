//
// Copyright (c) 2015 Mahyar Khayatkhoei
// Copyright (c) 2009-2014 Shawn Singh, Glen Berseth, Mubbasir Kapadia, Petros Faloutsos, Glenn Reinman
// See license.txt for complete license.
//

#include <algorithm>
#include <vector>
#include <util/Geometry.h>
#include <util/Curve.h>
#include <util/Color.h>
#include <util/DrawLib.h>
#include "Globals.h"

using namespace Util;

Curve::Curve(const CurvePoint& startPoint, int curveType) : type(curveType)
{
	controlPoints.push_back(startPoint);
}

Curve::Curve(const std::vector<CurvePoint>& inputPoints, int curveType) : type(curveType)
{
	controlPoints = inputPoints;
	sortControlPoints();
}

// Add one control point to the vector controlPoints
void Curve::addControlPoint(const CurvePoint& inputPoint)
{
	controlPoints.push_back(inputPoint);
	sortControlPoints();
}

// Add a vector of control points to the vector controlPoints
void Curve::addControlPoints(const std::vector<CurvePoint>& inputPoints)
{
	for (int i = 0; i < inputPoints.size(); i++)
		controlPoints.push_back(inputPoints[i]);
	sortControlPoints();
}

// Draw the curve shape on screen, using window as step size (bigger window: less accurate shape)
void Curve::drawCurve(Color curveColor, float curveThickness, int window)
{
#ifdef ENABLE_GUI
	curveThickness *= 2;
	window = 1; //Remove this line before submitting
	for (int i = 4; i < controlPoints.size(); i += window)
	{
		if (i + 1 < controlPoints.size())
		{
			DrawLib::drawLine(controlPoints[i].position, controlPoints[i + 1].position, curveColor, curveThickness);
		}
	}

	//std::cout << "\n window: " << window;
	
	// Robustness: make sure there is at least two control point: start and end points

	// Move on the curve from t=0 to t=finalPoint, using window as step size, and linearly interpolate the curve points
	
	return;
#endif
}

// Sort controlPoints vector in ascending order: min-first
void Curve::sortControlPoints()
{
	//Sort control points based on time it takes to travel from start point
	std::vector<CurvePoint> points = getControPoints(); //Control points in curve
	std::vector<int> distances(points.size()); //List of stored distances
	std::vector<CurvePoint> sortedPoints = getControPoints(); //List of sorted control points

	int numPoints = controlPoints.size();

	while (points.size() > 0)
	{
		Util::CurvePoint lo = points[0];
		int index = 0;

		for (int i = 0; i < points.size(); i++) //Loop through all points and find the one with the shortest distance
		{
			if (points[i].time < lo.time)
			{
				lo = points[i];
				index = i;
			}
		}

		//Add lo to the sorted list
		sortedPoints[numPoints - points.size()] = lo;

		//Remove lo from points
		points.erase(points.begin() + index);
	}

	/*
	//Print control points (check to make sure they are in order). Agent points are printed followed by camera points.
	std::cout << "Before sort\n";

	for (int i = 0; i < controlPoints.size(); i++)
	{
		std::cout << "Control Point: " << i << "Position: " << controlPoints[i].position << "Time: " << controlPoints[i].time << "\n";
	}

	controlPoints = sortedPoints;

	std::cout << "After sort\n";

	//Print control points (check to make sure they are in order)
	for (int i = 0; i < controlPoints.size(); i++)
	{
		std::cout << "Control Point: " << i << "Position: " << controlPoints[i].position << "Time: " << controlPoints[i].time << "\n";
	}
	*/

	return;
}

// Calculate the position on curve corresponding to the given time, outputPoint is the resulting position
bool Curve::calculatePoint(Point& outputPoint, float time)
{
	// Robustness: make sure there is at least two control point: start and end points
	if (!checkRobust())
		return false;
	
	// Define temporary parameters for calculation
	unsigned int nextPoint;
	float normalTime, intervalTime;

	// Find the current interval in time, supposing that controlPoints is sorted (sorting is done whenever control points are added)
	if (!findTimeInterval(nextPoint, time))
		return false;
	
	// Calculate position at t = time on curve
	if (type == hermiteCurve)
	{
		outputPoint = useHermiteCurve(nextPoint, time);
	}
	else if (type == catmullCurve)
	{
		outputPoint = useCatmullCurve(nextPoint, time);
	}

	// Return
	return true;
}

// Check Roboustness
bool Curve::checkRobust()
{
	//================DELETE THIS PART AND THEN START CODING===================
	static bool flag = false;
	if (!flag)
	{
		std::cerr << "ERROR>>>>Member function checkRobust is not implemented!" << std::endl;
		flag = true;
	}
	//=========================================================================


	return true;
}

// Find the current time interval (i.e. index of the next control point to follow according to current time)
bool Curve::findTimeInterval(unsigned int& nextPoint, float time)
{
	//Normalized time
	int maxTime = controlPoints[controlPoints.size()-1].time;
	
	float normalizedTime = time / maxTime;

	//Find out which control point is the current target
	float p0Interval = controlPoints[0].time / (maxTime); //0.0
	float p1Interval = controlPoints[1].time / (maxTime); //0.33
	float p2Interval = controlPoints[2].time / (maxTime); //0.66
	float p3Interval = controlPoints[3].time / (maxTime); //1.0

	std::vector<float> subcurveTime(controlPoints.size() - 1); //Vector holding the time it takes to reach each control point

	for (int i = 1; i < controlPoints.size(); i++)
	{
		subcurveTime[i - 1] = controlPoints[i].time;
	}

	for (int i = 0; i < subcurveTime.size(); i++) //Determine which is the next target controlpoint based on current time
	{
		if (time <= subcurveTime[nextPoint])
		{
			break;
		}
		else 
		{
			nextPoint++;
		}
	}


	return true;
}

// Implement Hermite curve
Point Curve::useHermiteCurve(const unsigned int nextPoint, const float time)
{
	Point newPosition = controlPoints[controlPoints.size() - 1].position; //Next position agent will be at
	float normalTime, intervalTime;
	Point endPoint = controlPoints[controlPoints.size() - 1].position;

	int maxTime = controlPoints[controlPoints.size() - 1].time; //Time it will take to finish entire traversal

	//Normalized time
	normalTime = time / maxTime;

	float totalCompleted = normalTime; //Location of agent in terms of entire curve (when normalTime = 1, curve is completed)

	float localCompleted; //Location of agent in terms of the current segment

	std::vector<float> subcurveTime(controlPoints.size()-1); //Vector holding the time it takes to reach each control point

	for (int i = 1; i < controlPoints.size(); i++)
	{
		subcurveTime[i-1] = controlPoints[i].time;
	}

	float prevSegTime = 0; //Amount of time it took to reach previous control point

	if (nextPoint == 0)
	{
		prevSegTime = 0;
	}
	else 
	{
		prevSegTime = subcurveTime[nextPoint - 1];
	}

	localCompleted = (time-prevSegTime) / (subcurveTime[nextPoint]-prevSegTime); //Point along local segment where agent must move next

	//std::cout << "\n traversed: " << totalCompleted << " segment " << subcurveTime[nextPoint] << " time " << time;

	//Get lerp time
	intervalTime = localCompleted;
	
	float t = intervalTime;
	float t2 = t*t;
	float t3 = t*t*t;

	// Calculate position at t = time on Hermite curve, tangents control curve height
	//Basis function
	float p1 = ((2)*(t3))-((3)*(t2))+(1);
	float p2 = ((-2)*(t3))+((3)*(t2));
	float h1 = ((t3)-((2)*(t2))+(t));
	float h2 = ((t3)-(t2));

	float curveFactor = time*0.5f;

	//std::cout << "\n cf: " << curveFactor;
	
	if (nextPoint == controlPoints.size())
	{
		newPosition = controlPoints[controlPoints.size() - 1].position;
	}
	else 
	{
		newPosition.x =
			p1*controlPoints[nextPoint].position.x +
			p2*controlPoints[nextPoint + 1].position.x +
			h1*controlPoints[nextPoint].tangent.x*curveFactor +
			h2*controlPoints[nextPoint + 1].tangent.x*curveFactor
			;

		newPosition.y =
			p1*controlPoints[nextPoint].position.y +
			p2*controlPoints[nextPoint + 1].position.y +
			h1*controlPoints[nextPoint].tangent.y*curveFactor +
			h2*controlPoints[nextPoint + 1].tangent.y*curveFactor
			;

		newPosition.z =
			p1*controlPoints[nextPoint].position.z +
			p2*controlPoints[nextPoint + 1].position.z +
			h1*controlPoints[nextPoint].tangent.z*curveFactor +
			h2*controlPoints[nextPoint + 1].tangent.z*curveFactor
			;
	}

	//Lock camera/agent to last point once time runs out
	if (nextPoint == controlPoints.size()-1 && time >= maxTime)
	{
		endPoint.x = -50;
		endPoint.y = 10;
		endPoint.z = 0;
		return endPoint;
	}

	CurvePoint myPoint = controlPoints[nextPoint];
	myPoint.position = newPosition;
	addControlPoint(myPoint);

	//std::cout << "\n pos: " << newPosition;
	return newPosition;
}

// Implement Catmull-Rom curve
Point Curve::useCatmullCurve(const unsigned int nextPoint, const float time)
{	
	Point newPosition = controlPoints[controlPoints.size() - 1].position; //Next position agent will be at
	float normalTime, intervalTime;
	Point endPoint = controlPoints[controlPoints.size() - 1].position;

	int point0 = nextPoint - 1;
	int point1 = nextPoint;
	int point2 = nextPoint+1;
	int point3 = nextPoint + 2;

	if (point0 < 0) { point0 = 0; } if (point0 > controlPoints.size()-1) { point0 = controlPoints.size() - 1; }
	if (point1 < 0) { point1 = 0; } if (point1 > controlPoints.size() - 1) { point1 = controlPoints.size() - 1; }
	if (point2 < 0) { point2 = 0; } if (point2 > controlPoints.size() - 1) { point2 = controlPoints.size() - 1; }
	if (point3 < 0) { point3 = 0; } if (point3 > controlPoints.size() - 1) { point3 = controlPoints.size() - 1; }

	int maxTime = controlPoints[controlPoints.size() - 1].time; //Time it will take to finish entire traversal

	//Normalized time
	normalTime = time / maxTime;

	float totalCompleted = normalTime; //Location of agent in terms of entire curve (when normalTime = 1, curve is completed)

	float localCompleted; //Location of agent in terms of the current segment

	std::vector<float> subcurveTime(controlPoints.size() - 1); //Vector holding the time it takes to reach each control point

	for (int i = 1; i < controlPoints.size(); i++)
	{
		subcurveTime[i - 1] = controlPoints[i].time;
	}

	float prevSegTime = 0; //Amount of time it took to reach previous control point

	if (nextPoint == 0)
	{
		prevSegTime = 0;
	}
	else
	{
		prevSegTime = subcurveTime[nextPoint - 1];
	}

	localCompleted = (time - prevSegTime) / (subcurveTime[nextPoint] - prevSegTime); //Point along local segment where agent must move next

	//Get lerp time
	intervalTime = localCompleted;

	float t = intervalTime;
	float t2 = t*t;
	float t3 = t*t*t;

	// Calculate position at t = time on Hermite curve, tangents control curve height
	//Basis function
	float p1 = 0.5f*(((-1)*(t3)) + ((2)*(t2)) - (t));
	float p2 = 0.5f*(((3)*(t3)) - ((5)*(t2)) + (2));
	float p3 = 0.5f*(((-3)*(t3)) + ((4)*(t2)) + (t));
	float p4 = 0.5f*((t3)-(t2));

	float curveFactor = (150 - controlPoints[nextPoint].time) / ((controlPoints[nextPoint + 1].time) - controlPoints[nextPoint].time);

	curveFactor = time;
	curveFactor /= 10;
	curveFactor = 1;

	newPosition =
		p1*controlPoints[point0].position +
		p2*controlPoints[point1].position +
		p3*controlPoints[point2].position +
		p4*controlPoints[point3].position
		;

	//Lock camera/agent to last point once time runs out
	if (nextPoint == controlPoints.size() - 1 && time >= maxTime)
	{
		endPoint.x = -50;
		endPoint.y = 10;
		endPoint.z = 0;
		return endPoint;
	}

	CurvePoint myPoint = controlPoints[nextPoint];
	myPoint.position = newPosition;
	addControlPoint(myPoint);

	//std::cout << "\n p0: " << point0 << " p1: " << point1 << " p2: " << point2 << " p3: " << point3;
	//std::cout << "\n pos: " << newPosition;
	return newPosition;
}