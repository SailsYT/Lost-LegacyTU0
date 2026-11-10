#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "BetaBiomeSource.h"
#include "SimplexNoiseOctaves.h"

// Voxel - was easier to just create a new one lol

Biome *BetaBiomeSource::biomeLookupTable[BetaBiomeSource::LOOKUP_ENTRIES];
bool BetaBiomeSource::lookupTableInitialised = false;

namespace
{
	template <typename T>
	void ensureCapacity(arrayWithLength<T> &array, unsigned int needed)
	{
		if (needed == 0)
		{
			if (array.data != nullptr) delete [] array.data;
			array.data = nullptr;
			array.length = 0;
			return;
		}
		if (array.data != nullptr && array.length >= needed) return;
		if (array.data != nullptr) delete [] array.data;
		array = arrayWithLength<T>(needed);
	}

	double clamp01(double value)
	{
		if (value < 0.0) return 0.0;
		if (value > 1.0) return 1.0;
		return value;
	}

	int javaRound(double value)
	{
		return static_cast<int>(floor(value + 0.5));
	}
}

BetaBiomeSource::BetaBiomeSource(int64_t seed)
{
	noiseGen1 = nullptr;
	noiseGen2 = nullptr;
	noiseGen3 = nullptr;
	this->seed = 0;

	_init();
	setSeed(seed);
}

BetaBiomeSource::~BetaBiomeSource()
{
	delete noiseGen1;
	delete noiseGen2;
	delete noiseGen3;
}

void BetaBiomeSource::setSeed(int64_t seed)
{
	if (this->seed == seed && noiseGen1 != nullptr) return;

	this->seed = seed;

	delete noiseGen1;
	delete noiseGen2;
	delete noiseGen3;

	Random temperatureRandom(static_cast<int64_t>(static_cast<uint64_t>(seed) * 9871ull));
	Random humidityRandom(static_cast<int64_t>(static_cast<uint64_t>(seed) * 39811ull));
	Random noiseRandom(static_cast<int64_t>(static_cast<uint64_t>(seed) * 543321ull));

	noiseGen1 = new SimplexNoiseOctaves(&temperatureRandom, 4);
	noiseGen2 = new SimplexNoiseOctaves(&humidityRandom, 4);
	noiseGen3 = new SimplexNoiseOctaves(&noiseRandom, 2);
}

void BetaBiomeSource::staticCtor()
{
	ensureLookupTable();
}

void BetaBiomeSource::ensureLookupTable()
{
	if (lookupTableInitialised) return;

	for (int humidity = 0; humidity < LOOKUP_SIZE; humidity++)
	{
		for (int temperature = 0; temperature < LOOKUP_SIZE; temperature++)
		{
			biomeLookupTable[temperature + humidity * LOOKUP_SIZE] = getBiome(temperature / 63.0f, humidity / 63.0f);
		}
	}

	lookupTableInitialised = true;
}

Biome *BetaBiomeSource::getBiome(float temperature, float humidity)
{
	const float humidityTimesTemperature = humidity * temperature;

	if (humidityTimesTemperature < 0.2f)
	{
		if (temperature < 0.5f) return Biome::iceFlats;
		if (temperature < 0.95f) return Biome::plains;
		return Biome::desert;
	}

	if (temperature < 0.5f) return Biome::taiga;

	if (temperature < 0.95f)
	{
		if (humidityTimesTemperature > 0.8f) return Biome::swampland;
		return Biome::forest;
	}

	if (humidityTimesTemperature > 0.8f) return Biome::rainforest;
	return Biome::plains;
}

Biome *BetaBiomeSource::getBiomeFromLookup(double temperature, double humidity)
{
	ensureLookupTable();

	int temperatureIndex = javaRound(temperature * 63.0);
	int humidityIndex = javaRound(humidity * 63.0);

	if (temperatureIndex < 0) temperatureIndex = 0;
	if (temperatureIndex >= LOOKUP_SIZE) temperatureIndex = LOOKUP_SIZE - 1;
	if (humidityIndex < 0) humidityIndex = 0;
	if (humidityIndex >= LOOKUP_SIZE) humidityIndex = LOOKUP_SIZE - 1;

	return biomeLookupTable[temperatureIndex + humidityIndex * LOOKUP_SIZE];
}

void BetaBiomeSource::getClimateBlock(doubleArray &temperatures, doubleArray &humidities, int x, int z, int w, int h) const
{
	const unsigned int needed = static_cast<unsigned int>(w) * static_cast<unsigned int>(h);
	ensureCapacity(temperatures, needed);
	ensureCapacity(humidities, needed);

	doubleArray temperatureNoise = noiseGen1->getRegion(doubleArray(), x, z, w, h, 0.025, 0.025, 0.25);
	doubleArray humidityNoise = noiseGen2->getRegion(doubleArray(), x, z, w, h, 0.05, 0.05, 0.3333333333333333);
	doubleArray climateNoise = noiseGen3->getRegion(doubleArray(), x, z, w, h, 0.025, 0.025, 0.5882352941176471);

	for (unsigned int i = 0; i < needed; i++)
	{
		double large = climateNoise[i] * 1.1 + 0.5;

		double humidityMix = 0.01;
		double temperature = (temperatureNoise[i] * 0.15 + 0.7) * (1.0 - humidityMix) + large * humidityMix;

		double temperatureMix = 0.002;
		double humidity = (humidityNoise[i] * 0.15 + 0.5) * (1.0 - temperatureMix) + large * temperatureMix;

		temperature = 1.0 - (1.0 - temperature) * (1.0 - temperature);

		temperatures[i] = clamp01(temperature);
		humidities[i] = clamp01(humidity);
	}

	delete [] temperatureNoise.data;
	delete [] humidityNoise.data;
	delete [] climateNoise.data;
}

void BetaBiomeSource::getBiomeIndicesInternal(intArray &indices, int x, int z, int w, int h) const
{
	const unsigned int needed = static_cast<unsigned int>(w) * static_cast<unsigned int>(h);
	ensureCapacity(indices, needed);

	doubleArray temperatures;
	doubleArray humidities;
	getClimateBlock(temperatures, humidities, x, z, w, h);

	for (unsigned int i = 0; i < needed; i++)
	{
		indices[i] = getBiomeFromLookup(temperatures[i], humidities[i])->id;
	}

	delete [] temperatures.data;
	delete [] humidities.data;
}

Biome *BetaBiomeSource::getBiome(ChunkPos *cp)
{
	return getBiome(cp->x << 4, cp->z << 4);
}

Biome *BetaBiomeSource::getBiome(int x, int z)
{
	doubleArray temperatures;
	doubleArray humidities;
	getClimateBlock(temperatures, humidities, x, z, 1, 1);

	Biome *result = getBiomeFromLookup(temperatures[0], humidities[0]);

	delete [] temperatures.data;
	delete [] humidities.data;
	return result;
}

float BetaBiomeSource::getDownfall(int x, int z) const
{
	doubleArray temperatures;
	doubleArray humidities;
	getClimateBlock(temperatures, humidities, x, z, 1, 1);

	float result = static_cast<float>(humidities[0]);

	delete [] temperatures.data;
	delete [] humidities.data;
	return result;
}

floatArray BetaBiomeSource::getDownfallBlock(int x, int z, int w, int h) const
{
	floatArray downfalls;
	getDownfallBlock(downfalls, x, z, w, h);
	return downfalls;
}

void BetaBiomeSource::getDownfallBlock(floatArray &downfalls, int x, int z, int w, int h) const
{
	const unsigned int needed = static_cast<unsigned int>(w) * static_cast<unsigned int>(h);
	ensureCapacity(downfalls, needed);

	doubleArray temperatures;
	doubleArray humidities;
	getClimateBlock(temperatures, humidities, x, z, w, h);

	for (unsigned int i = 0; i < needed; i++)
	{
		downfalls[i] = static_cast<float>(humidities[i]);
	}

	delete [] temperatures.data;
	delete [] humidities.data;
}

float BetaBiomeSource::getTemperature(int x, int y, int z) const
{
	doubleArray temperatures;
	doubleArray humidities;
	getClimateBlock(temperatures, humidities, x, z, 1, 1);

	float result = scaleTemp(static_cast<float>(temperatures[0]), y);

	delete [] temperatures.data;
	delete [] humidities.data;
	return result;
}

floatArray BetaBiomeSource::getTemperatureBlock(int x, int z, int w, int h) const
{
	floatArray temperatures;
	getTemperatureBlock(temperatures, x, z, w, h);
	return temperatures;
}

void BetaBiomeSource::getTemperatureBlock(floatArray &temperatures, int x, int z, int w, int h) const
{
	const unsigned int needed = static_cast<unsigned int>(w) * static_cast<unsigned int>(h);
	ensureCapacity(temperatures, needed);

	doubleArray temperatureNoise;
	doubleArray humidities;
	getClimateBlock(temperatureNoise, humidities, x, z, w, h);

	for (unsigned int i = 0; i < needed; i++)
	{
		temperatures[i] = static_cast<float>(temperatureNoise[i]);
	}

	delete [] temperatureNoise.data;
	delete [] humidities.data;
}

BiomeArray BetaBiomeSource::getRawBiomeBlock(int x, int z, int w, int h) const
{
	BiomeArray biomes;
	getRawBiomeBlock(biomes, x, z, w, h);
	return biomes;
}

void BetaBiomeSource::getRawBiomeBlock(BiomeArray &biomes, int x, int z, int w, int h) const
{
	const unsigned int needed = static_cast<unsigned int>(w) * static_cast<unsigned int>(h);
	ensureCapacity(biomes, needed);

	intArray indices;
	getBiomeIndicesInternal(indices, x, z, w, h);

	for (unsigned int i = 0; i < needed; i++)
	{
		biomes[i] = Biome::biomes[indices[i]];
	}

	delete [] indices.data;
}

void BetaBiomeSource::getRawBiomeIndices(intArray &biomes, int x, int z, int w, int h) const
{
	const unsigned int needed = static_cast<unsigned int>(w) * static_cast<unsigned int>(h);
	ensureCapacity(biomes, needed);

	intArray indices;
	getBiomeIndicesInternal(indices, x, z, w, h);

	for (unsigned int i = 0; i < needed; i++)
	{
		biomes[i] = indices[i];
	}

	delete [] indices.data;
}

BiomeArray BetaBiomeSource::getBiomeBlock(int x, int z, int w, int h) const
{
	BiomeArray biomes;
	getBiomeBlock(biomes, x, z, w, h, true);
	return biomes;
}

void BetaBiomeSource::getBiomeBlock(BiomeArray &biomes, int x, int z, int w, int h, bool useCache) const
{
	getRawBiomeBlock(biomes, x, z, w, h);
}

byteArray BetaBiomeSource::getBiomeIndexBlock(int x, int z, int w, int h) const
{
	byteArray biomeIndices;
	getBiomeIndexBlock(biomeIndices, x, z, w, h, true);
	return biomeIndices;
}

void BetaBiomeSource::getBiomeIndexBlock(byteArray &biomeIndices, int x, int z, int w, int h, bool useCache) const
{
	const unsigned int needed = static_cast<unsigned int>(w) * static_cast<unsigned int>(h);
	ensureCapacity(biomeIndices, needed);

	intArray indices;
	getBiomeIndicesInternal(indices, x, z, w, h);

	for (unsigned int i = 0; i < needed; i++)
	{
		biomeIndices[i] = static_cast<byte>(indices[i]);
	}

	delete [] indices.data;
}

bool BetaBiomeSource::containsOnly(int x, int z, int r, vector<Biome *> allowed)
{
	int x0 = x - r;
	int z0 = z - r;
	int w = r * 2 + 1;
	int h = r * 2 + 1;

	intArray indices;
	getBiomeIndicesInternal(indices, x0, z0, w, h);

	bool result = true;
	const unsigned int needed = static_cast<unsigned int>(w) * static_cast<unsigned int>(h);
	for (unsigned int i = 0; i < needed; i++)
	{
		Biome *biome = Biome::biomes[indices[i]];
		if (find(allowed.begin(), allowed.end(), biome) == allowed.end())
		{
			result = false;
			break;
		}
	}

	delete [] indices.data;
	return result;
}

bool BetaBiomeSource::containsOnly(int x, int z, int r, Biome *allowed)
{
	int x0 = x - r;
	int z0 = z - r;
	int w = r * 2 + 1;
	int h = r * 2 + 1;

	intArray indices;
	getBiomeIndicesInternal(indices, x0, z0, w, h);

	bool result = true;
	const unsigned int needed = static_cast<unsigned int>(w) * static_cast<unsigned int>(h);
	for (unsigned int i = 0; i < needed; i++)
	{
		if (Biome::biomes[indices[i]] != allowed)
		{
			result = false;
			break;
		}
	}

	delete [] indices.data;
	return result;
}

TilePos *BetaBiomeSource::findBiome(int x, int z, int r, Biome *toFind, Random *random)
{
	vector<Biome *> allowed;
	allowed.push_back(toFind);
	return findBiome(x, z, r, allowed, random);
}

TilePos *BetaBiomeSource::findBiome(int x, int z, int r, vector<Biome *> allowed, Random *random)
{
	int x0 = x - r;
	int z0 = z - r;
	int w = r * 2 + 1;
	int h = r * 2 + 1;

	intArray indices;
	getBiomeIndicesInternal(indices, x0, z0, w, h);

	TilePos *result = nullptr;
	int found = 0;
	for (int zz = z0; zz <= z + r; zz++)
	{
		for (int xx = x0; xx <= x + r; xx++)
		{
			Biome *biome = Biome::biomes[indices[(xx - x0) + (zz - z0) * w]];
			if (find(allowed.begin(), allowed.end(), biome) != allowed.end())
			{
				if (result == nullptr || random->nextInt(found + 1) == 0)
				{
					delete result;
					result = new TilePos(xx, 0, zz);
					found++;
				}
			}
		}
	}

	delete [] indices.data;
	return result;
}
