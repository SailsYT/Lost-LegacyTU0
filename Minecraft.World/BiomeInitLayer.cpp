#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.newbiome.layer.h"
#include "net.minecraft.world.level.h"
#include "BiomeInitLayer.h"

BiomeInitLayer::BiomeInitLayer(int64_t seed, shared_ptr<Layer>parent, LevelType *levelType) : Layer(seed)
{
	this->parent = parent;

	if(levelType == LevelType::lvl_normal_1_1)
	{
		startBiomes = BiomeArray(5);
		startBiomes[0] = Biome::desert;
		startBiomes[1] = Biome::forest;
		startBiomes[2] = Biome::swampland;
		startBiomes[3] = Biome::plains;
		startBiomes[4] = Biome::taiga;
	}
	else
	{
		startBiomes = BiomeArray(6);
		startBiomes[0] = Biome::desert;
		startBiomes[1] = Biome::forest;
		startBiomes[2] = Biome::swampland;
		startBiomes[3] = Biome::plains;
		startBiomes[4] = Biome::taiga;
		startBiomes[5] = Biome::rainforest;
	}
}

BiomeInitLayer::~BiomeInitLayer()
{
	delete [] startBiomes.data;
}

intArray BiomeInitLayer::getArea(int xo, int yo, int w, int h)
{
	intArray b = parent->getArea(xo, yo, w, h);

	intArray result = IntCache::allocate(w * h);
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			initRandom(x + xo, y + yo);
            int old = b[x + y * w];
            if (old == 0)
			{
                result[x + y * w] = 0;
            }
			else if (old == 1)
			{
                result[x + y * w] = startBiomes[nextRandom(startBiomes.length)]->id;
            }
			else
			{
				int isTaiga = startBiomes[nextRandom(startBiomes.length)]->id;
				if (isTaiga == Biome::taiga->id)
				{
					result[x + y * w] = isTaiga;
				}
				else
				{
					result[x + y * w] = Biome::iceFlats->id;
				}
			}
		}
	}

	return result;
}