#pragma once
#ifndef __FEATURE__
#define __FEATURE__
typedef struct Feature
{
	int octave;
	int level;
	float scale, nscale, x, y, value;

	int nx, ny;

	float orientation;
	float vec[128];

	Feature* nearest;

	float minDist1;
	float minDist2;

	Feature() {}

	Feature(int o, int l, float x, float y, float v)
		:octave(o), level(l), x(x), y(y), value(v), minDist1(FLT_MAX), minDist2(FLT_MAX)
	{
		if (!octave)
		{
			nx = (int)x >> 1;
			ny = (int)y >> 1;
		}
		else
		{
			nx = (int)x << (octave - 1);
			ny = (int)y << (octave - 1);
		}
		nscale = (float)l;
		scale = nscale * (float)(1 << o);
	}

	Feature(int o, int l, int x, int y, float v)
		:octave(o), level(l), x((float)x), y((float)y), value(v), minDist1(FLT_MAX), minDist2(FLT_MAX)
	{
		if (!octave)
		{
			nx = (int)x >> 1;
			ny = (int)y >> 1;
		}
		else
		{
			nx = (int)x << (octave - 1);
			ny = (int)y << (octave - 1);
		}
		nscale = (float)l;
		scale = nscale * (float)(1 << o);
	}

	Feature(const Feature& key, float ori)
		:octave(key.octave), level(key.level), scale(key.scale), nscale(key.nscale), x(key.x), y(key.y), value(key.value), nx(key.nx), ny(key.ny), orientation(ori), minDist1(FLT_MAX), minDist2(FLT_MAX)
	{}

	Feature(const Feature& key)
		:octave(key.octave), level(key.level), scale(key.scale), nscale(key.nscale), x(key.x), y(key.y), value(key.value), nx(key.nx), ny(key.ny), orientation(key.orientation), minDist1(FLT_MAX), minDist2(FLT_MAX)
	{}
}feature_t;
#endif

float _CalcSIFTSqDist(const feature_t& k1, const feature_t& k2);
