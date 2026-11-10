#pragma once

class SimplexNoise;

class SimplexNoiseOctaves
{
private:
	SimplexNoise **generatorCollection;
	int octaves;

public:
	SimplexNoiseOctaves(Random *random, int octaves);
	~SimplexNoiseOctaves();

	doubleArray getRegion(doubleArray buffer, double x, double z, int xSize, int zSize, double xScale, double zScale, double lacunarity) const;
	doubleArray getRegion(doubleArray buffer, double x, double z, int xSize, int zSize, double xScale, double zScale, double lacunarity, double persistence) const;
};
