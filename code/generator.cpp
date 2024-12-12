#include "generator.h"

OctaveGenerator::OctaveGenerator(std::function<float(float, float)> noise) : noise{ noise } {}

void OctaveGenerator::reconfigure(bool _regen, int octave, int seed) {
    this->_regen = _regen;
    this->octave = octave;
    this->seed = seed;
}

void OctaveGenerator::regenSeeds() {
    seedOffsetX.clear();
    seedOffsetY.clear();
    ofSeedRandom(seed);
    for (int i = 0; i < octave; ++i) {
        seedOffsetX.push_back(ofRandom(255));
        seedOffsetY.push_back(ofRandom(255));
    }
}

std::vector<std::vector<float>> OctaveGenerator::generate(size_t size) {
    std::vector<std::vector<float>> heightMap;
    for (int y = 0; y <= size; y++) {
        vector<float> row;
        for (int x = 0; x <= size; x++)
            row.push_back(0);
        heightMap.push_back(row);
    }
    float x, y, z;
    float freq, ampl;

    if (_regen) {
        regenSeeds();
    }
    minNoise = 1;
    maxNoise = 0;
    for (int i = 0; i <= size; ++i) {
        for (int j = 0; j <= size; ++j) {
            freq = 1;
            ampl = 1;
            z = 0;

            for (int o = 0; o < octave; ++o) {
                x = ((float)j - size / 2) / size / scale * freq + seedOffsetX.at(o);
                y = ((float)i - size / 2) / size / scale * freq + seedOffsetY.at(o);
                z += noise(x, y) * ampl;

                freq *= lacunarity;
                ampl *= persistence;
            }

            if (z < minNoise) {
                minNoise = z;
            }
            if (z > maxNoise) {
                maxNoise = z;
            }
            heightMap.at(i).at(j) = z;
        }
    }
    return heightMap;
}

float OctaveGenerator::getMinNoise() {
    return minNoise;
}

float OctaveGenerator::getMaxNoise() {
    return maxNoise;
}