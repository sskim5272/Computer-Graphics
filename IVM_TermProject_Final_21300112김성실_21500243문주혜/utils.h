#pragma once

#include <GL/glew.h>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

typedef std::vector<GLfloat> GLvec;

// Grid vertex
void get_grid(std::vector<GLfloat>& p)
{
	float start = -2.5f;                float range = 0.5f;
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			p.push_back(start + range *  j);        p.push_back(0);     p.push_back(start + range *  i);
			p.push_back(start + range * (j + 1));   p.push_back(0);     p.push_back(start + range *  i);
			p.push_back(start + range * (j + 1));   p.push_back(0);     p.push_back(start + range * (i + 1));
			p.push_back(start + range *  j);        p.push_back(0);     p.push_back(start + range * (i + 1));
		}
	}
}