#pragma once
#include <vector>
#include <functional>
#include <memory>
#include "ofMain.h"

class HeightGenerator {
public:
	/*
		Pre-allocated squared matrix of size 'size' expected.
	*/
	virtual std::vector<std::vector<float>> generate(size_t size) = 0;
	virtual void reconfigure(bool _regen = true, int octave = 8, int seed = 0) = 0;
	virtual float getMinNoise() = 0;
	virtual float getMaxNoise() = 0;
	virtual ~HeightGenerator() = default;
};

class OctaveGenerator : public HeightGenerator {
private:
	const float scale = 1.0f;
	const float lacunarity = 2.0f;
	const float persistence = 0.5f;
	bool _regen = true;
	int octave = 8;
	int seed = 0;
	float minNoise = 1;
	float maxNoise = 0;

	std::vector<float> seedOffsetX;
	std::vector<float> seedOffsetY;
	std::function<float(float, float)> noise;

	void regenSeeds();

public:
	OctaveGenerator(std::function<float(float, float)> noise);
	void reconfigure(bool _regen = true, int octave = 8, int seed = 0) override;
	std::vector<std::vector<float>> generate(size_t size) override;
	float getMinNoise() override;
	float getMaxNoise() override;
};

class HeightGenerator_Creator {
public:
	virtual std::unique_ptr<HeightGenerator> create(std::function<float(float, float)> noise) = 0;
	virtual ~HeightGenerator_Creator() = default;
};

class OctaveGenerator_Creator : public HeightGenerator_Creator {
	virtual std::unique_ptr<HeightGenerator> create(std::function<float(float, float)> noise) override {
		return std::make_unique<OctaveGenerator>(std::move(noise));
	}
};