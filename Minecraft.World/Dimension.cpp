#include "stdafx.h"
#include "net.minecraft.world.level.levelgen.flat.h"
#include "net.minecraft.world.level.levelgen.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.storage.h"
#include "Dimension.h"
#include "BiomeSource.h"
#include "BetaBiomeSource.h"
#include "FixedBiomeSource.h"
#include "OldChunkStorage.h"
#include "HellDimension.h"
#include "NormalDimension.h"
#include "TheEndDimension.h"
#include "net.minecraft.world.level.tile.h"
#include "../Minecraft.Client/Minecraft.h"
#include "../Minecraft.Client/Common/Colours/ColourTable.h"

const float Dimension::MOON_BRIGHTNESS_PER_PHASE[8] = {1.0f, 0.75f, 0.5f, 0.25f, 0, 0.25f, 0.5f, 0.75f};

void Dimension::init(Level *level)
{
	this->level = level;
	levelType = level->getLevelData()->getGenerator();
	levelTypeOptions = level->getLevelData()->getGeneratorOptions();
	init();
	updateLightRamp();
}

void Dimension::updateLightRamp()
{
	float ambientLight = (id == -1) ? 0.1f : 0.05f;
	for (int i = 0; i <= Level::MAX_BRIGHTNESS; i++)
	{
		float v = (1 - i / static_cast<float>(Level::MAX_BRIGHTNESS));
		brightnessRamp[i] = ((1 - v) / (v * 3 + 1)) * (1 - ambientLight) + ambientLight;
	}
}

void Dimension::init()
{
#ifdef _OVERRIDE_HEIGHTMAP
	// 4J Stu - Added to enable overriding the heightmap from a loaded in data file
	if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_EnableBiomeOverride))
	{
		biomeSource = new BiomeSource(level);
	}
	else
#endif
		if (level->getLevelData()->getGenerator() == LevelType::lvl_flat)
		{
			FlatGeneratorInfo *generator = FlatGeneratorInfo::fromValue(level->getLevelData()->getGeneratorOptions());
			biomeSource = new FixedBiomeSource(Biome::biomes[generator->getBiome()], 0.5f, 0.5f);
			delete generator;
		}
		else
		{
			biomeSource = new BetaBiomeSource(level->getSeed());
		}
}

Dimension::Dimension()
{
	ultraWarm = false;
	hasCeiling = false;
	brightnessRamp = new float[Level::MAX_BRIGHTNESS + 1];
	id = 0;
	levelTypeOptions = L"";
}

Dimension::~Dimension()
{
	delete[] brightnessRamp;

	if(biomeSource != nullptr)
		delete biomeSource;
}

ChunkSource *Dimension::createRandomLevelSource() const
{
#ifdef _OVERRIDE_HEIGHTMAP
	// 4J Stu - Added to enable overriding the heightmap from a loaded in data file
	if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_EnableHeightWaterOverride))
	{
		return new CustomLevelSource(level, level->getSeed(), level->getLevelData()->isGenerateMapFeatures());
	}
	else
#endif
		if (levelType == LevelType::lvl_flat)
		{
			return new FlatLevelSource(level, level->getSeed(), level->getLevelData()->isGenerateMapFeatures());
		}
		else
		{
			return new RandomLevelSource(level, level->getSeed(), level->getLevelData()->isGenerateMapFeatures());
		}
}

ChunkSource *Dimension::createFlatLevelSource() const
{
	return new FlatLevelSource(level, level->getSeed(), level->getLevelData()->isGenerateMapFeatures());
}

ChunkStorage *Dimension::createStorage(File dir)
{
	return new OldChunkStorage(dir, true);
}

bool Dimension::isValidSpawn(int x, int z) const
{
	int topTile = level->getTopTile(x, z);

	if (topTile != Tile::grass_Id) return false;

	return true;
}

float Dimension::getTimeOfDay(int64_t time, float a) const
{
	int dayStep = static_cast<int>(time % Level::TICKS_PER_DAY);
	float td = (dayStep + a) / Level::TICKS_PER_DAY - 0.25f;
	if (td < 0) td += 1;
	if (td > 1) td -= 1;
	float tdo = td;
	td = 1 - (float) ((cos(td * PI) + 1) / 2);
	td = tdo + (td - tdo) / 3.0f;
	return td;
}

int Dimension::getMoonPhase(int64_t time) const
{
	return static_cast<int>(time / Level::TICKS_PER_DAY) % 8;
}

bool Dimension::isNaturalDimension()
{
	return true;
}

float *Dimension::getSunriseColor(float td, float a)
{
	float span = 0.4f;
	float tt = Mth::cos(td * PI * 2) - 0.0f;
	float mid = -0.0f;
	if (tt >= mid - span && tt <= mid + span)
	{
		float aa = ((tt - mid) / span) * 0.5f + 0.5f;
		float mix = 1 - (((1 - sin(aa * PI))) * 0.99f);
		mix = mix * mix;

		sunriseCol[0] = (aa * 0.3f + 0.7f);
		sunriseCol[1] = (aa * aa * 0.7f + 0.2f);
		sunriseCol[2] = (aa * aa * 0.0f + 0.2f); 
		sunriseCol[3] = mix;
		return sunriseCol;
	}

	return nullptr;
}

Vec3 *Dimension::getFogColor(float td, float a) const
{
	float br = Mth::cos(td * 3.1415927f * 2.0f) * 2.0f + 0.5f;
	if (br < 0.0f) br = 0.0f;
	if (br > 1.0f) br = 1.0f;

	unsigned int baseFogColour = 0xC0D8FF;
	float r = ((baseFogColour >> 16) & 0xff) / 255.0f;
	float g = ((baseFogColour >> 8) & 0xff) / 255.0f;
	float b = ((baseFogColour) & 0xff) / 255.0f;
	r *= br * 0.94f + 0.06f;
	g *= br * 0.94f + 0.06f;
	b *= br * 0.91f + 0.09f;

	return Vec3::newTemp(r, g, b);
}

bool Dimension::mayRespawn() const
{
	return true;
}

Dimension *Dimension::getNew(int id)
{
	if (id == -1) return new HellDimension();
	if (id == 0) return new NormalDimension();
	if (id == 1) return new TheEndDimension();

	return nullptr;
}

float Dimension::getCloudHeight()
{
	return static_cast<float>(Level::genDepth);
}

bool Dimension::hasGround()
{
	return true;
}

Pos *Dimension::getSpawnPos()
{
	return nullptr;
}

int Dimension::getSpawnYPosition()
{
	if (levelType == LevelType::lvl_flat)
	{
		return 4;
	}
	return Level::genDepth / 2;
}

bool Dimension::hasBedrockFog()
{
	// 4J-PB - turn off bedrock fog if the host player doesn't want it
	if(app.GetGameHostOption(eGameHostOption_BedrockFog)==0)
	{
		return false;
	}

	return (levelType != LevelType::lvl_flat && !hasCeiling);
}

double Dimension::getClearColorScale()
{
	if (levelType == LevelType::lvl_flat)
	{
		return 1.0;
	}
	return 1.0 / 32.0;
}

bool Dimension::isFoggyAt(int x, int z)
{
	return false;
}

int Dimension::getXZSize()
{
	return level->getLevelData()->getXZSize();
}
