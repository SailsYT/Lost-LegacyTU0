#pragma once
#include "BiomeSource.h"

class SimplexNoiseOctaves;

class BetaBiomeSource : public BiomeSource
{
public:
	static const int LOOKUP_SIZE = 64;
	static const int LOOKUP_ENTRIES = LOOKUP_SIZE * LOOKUP_SIZE;

private:
	static Biome *biomeLookupTable[LOOKUP_ENTRIES];
	static bool lookupTableInitialised;

	SimplexNoiseOctaves *noiseGen1;
	SimplexNoiseOctaves *noiseGen2;
	SimplexNoiseOctaves *noiseGen3;

	int64_t seed;

	void setSeed(int64_t seed);

	static void ensureLookupTable();
	static Biome *getBiomeFromLookup(double temperature, double humidity);
	static Biome *getBiome(float temperature, float humidity);

	void getBiomeIndicesInternal(intArray &indices, int x, int z, int w, int h) const;

public:
	static void staticCtor();

	virtual void getClimateBlock(doubleArray &temperatures, doubleArray &humidities, int x, int z, int w, int h) const;

	BetaBiomeSource(int64_t seed);
	virtual ~BetaBiomeSource();

	virtual Biome *getBiome(ChunkPos *cp);
	virtual Biome *getBiome(int x, int z);

	virtual float getDownfall(int x, int z) const;
	virtual floatArray getDownfallBlock(int x, int z, int w, int h) const;
	virtual void getDownfallBlock(floatArray &downfalls, int x, int z, int w, int h) const;

	virtual float getTemperature(int x, int y, int z) const;
	virtual floatArray getTemperatureBlock(int x, int z, int w, int h) const;
	virtual void getTemperatureBlock(floatArray &temperatures, int x, int z, int w, int h) const;

	virtual BiomeArray getRawBiomeBlock(int x, int z, int w, int h) const;
	virtual void getRawBiomeBlock(BiomeArray &biomes, int x, int z, int w, int h) const;
	virtual void getRawBiomeIndices(intArray &biomes, int x, int z, int w, int h) const;

	virtual BiomeArray getBiomeBlock(int x, int z, int w, int h) const;
	virtual void getBiomeBlock(BiomeArray &biomes, int x, int z, int w, int h, bool useCache) const;

	virtual byteArray getBiomeIndexBlock(int x, int z, int w, int h) const;
	virtual void getBiomeIndexBlock(byteArray &biomeIndices, int x, int z, int w, int h, bool useCache) const;

	virtual bool containsOnly(int x, int z, int r, vector<Biome *> allowed);
	virtual bool containsOnly(int x, int z, int r, Biome *allowed);

	virtual TilePos *findBiome(int x, int z, int r, Biome *toFind, Random *random);
	virtual TilePos *findBiome(int x, int z, int r, vector<Biome *> allowed, Random *random);
};
