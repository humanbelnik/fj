#pragma once

#include "ofMain.h"
#include "generator.h"
#include <vector>
#include <memory>

class Fjord {
private:
	int maxElevation = 3000;
	int tileSize = 20;
	int size = 10000;
	float flatten = 1.0f;

	bool isLake = false;
	float waterPercentage = 0.5f;

	std::vector<std::vector<float>> heightMap;
	std::unique_ptr<HeightGenerator> generator;

	void initHeightMap();
	void applyMapType();

public:
	Fjord(std::unique_ptr<HeightGenerator> generator);
	void update(bool _regen = true, int octave = 8, int seed = 0,
		int maxElevation = 3000, int tileSize = 50, bool isLake = false, float waterPercentage = 0.5);
	std::vector<std::vector<float>> getHeightMap();
	int getSize();
	int getTileSize();
	int getMaxElevation();
};