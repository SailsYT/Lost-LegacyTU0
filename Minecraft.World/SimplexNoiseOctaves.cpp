#include "stdafx.h"
#include "SimplexNoiseOctaves.h"
#include "SimplexNoise.h"

SimplexNoiseOctaves::SimplexNoiseOctaves(Random *random, int octaves)
{
	this->octaves = octaves;
	generatorCollection = new SimplexNoise *[octaves];
	for (int i = 0; i < octaves; i++)
	{
		generatorCollection[i] = new SimplexNoise(random);
	}
}

SimplexNoiseOctaves::~SimplexNoiseOctaves()
{
	for (int i = 0; i < octaves; i++)
	{
		delete generatorCollection[i];
	}
	delete [] generatorCollection;
}

doubleArray SimplexNoiseOctaves::getRegion(doubleArray buffer, double x, double z, int xSize, int zSize, double xScale, double zScale, double lacunarity) const
{
	return getRegion(buffer, x, z, xSize, zSize, xScale, zScale, lacunarity, 0.5);
}

doubleArray SimplexNoiseOctaves::getRegion(doubleArray buffer, double x, double z, int xSize, int zSize, double xScale, double zScale, double lacunarity, double persistence) const
{
	const unsigned int needed = static_cast<unsigned int>(xSize) * static_cast<unsigned int>(zSize);

	xScale /= 1.5;
	zScale /= 1.5;

	if (buffer.data != nullptr && buffer.length >= needed)
	{
		for (unsigned int i = 0; i < buffer.length; i++)
		{
			buffer[i] = 0;
		}
	}
	else
	{
		if (buffer.data != nullptr) delete [] buffer.data;
		buffer = doubleArray(needed);
	}

	double frequency = 1.0;
	double amplitude = 1.0;
	for (int i = 0; i < octaves; i++)
	{
		generatorCollection[i]->add(buffer, x, z, xSize, zSize, xScale * frequency, zScale * frequency, 0.55 / amplitude);
		frequency *= lacunarity;
		amplitude *= persistence;
	}

	return buffer;
}
