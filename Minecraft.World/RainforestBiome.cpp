#include "stdafx.h"
#include "RainforestBiome.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.levelgen.feature.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.animal.h"

RainforestBiome::RainforestBiome(int id) : Biome(id)
{
	decorator->treeCount = 50;
	decorator->grassCount = 25;
	decorator->flowerCount = 4;
}

Feature *RainforestBiome::getTreeFeature(Random *random)
{
    if (random->nextInt(3) == 0)
	{
        return new BasicTree(false);
    }
    return new TreeFeature(false);
}

Feature *RainforestBiome::getGrassFeature(Random *random)
{
	if (random->nextInt(4) == 0)
	{
		return new TallGrassFeature(Tile::tallgrass_Id, TallGrass::FERN);
	}
	return new TallGrassFeature(Tile::tallgrass_Id, TallGrass::TALL_GRASS);
}