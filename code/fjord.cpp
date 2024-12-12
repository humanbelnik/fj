#include "fjord.h"

Fjord::Fjord(std::unique_ptr<HeightGenerator>generator) : generator{ std::move(generator) } {}

void Fjord::update(bool _regen, int octave, int seed,
    int maxElevation, int tileSize, bool isLake, float waterPercentage) {
    /*
        Update settings
    */
    this->size = 10000;
    generator->reconfigure(_regen, octave, seed);
    this->maxElevation = maxElevation;
    this->tileSize = tileSize;
    this->isLake = isLake;
    this->waterPercentage = waterPercentage;
    this->size /= this->tileSize;

    /*
        Apply settings
    */
    heightMap = generator->generate(this->size);
    this->applyMapType();

}

void Fjord::initHeightMap() {
    heightMap.clear();
    for (int y = 0; y <= size; y++) {
        vector<float> row;
        for (int x = 0; x <= size; x++)
            row.push_back(0);
        heightMap.push_back(row);
    }
}


void Fjord::applyMapType() {
    float maxEuclideanDistance = pow(waterPercentage, 0.5);
    float minNoise = generator->getMinNoise();
    float maxNoise = generator->getMaxNoise();

    for (int y = 0; y <= size; ++y) {
        for (int x = 0; x <= size; ++x) {
            if (!isLake) {
                float noiseValue = heightMap.at(y).at(x);
                noiseValue = ofMap(noiseValue, minNoise, maxNoise, 0, 1);
                noiseValue = pow(noiseValue, flatten);
                heightMap.at(y).at(x) = ofMap(noiseValue, 0, 1, -maxElevation, maxElevation);
                if (heightMap.at(y).at(x) < 0.0f)
                    heightMap.at(y).at(x) = 0.0f;
            }
            else {
                float euclideanDistance = sqrt(pow((float)x / size - 0.5f, 2) + pow((float)y / size - 0.5f, 2));
                float noiseValue = heightMap.at(y).at(x);
                noiseValue = ofMap(noiseValue, minNoise, maxNoise, 0, 1);

                if (waterPercentage > 0.95f) {
                    heightMap.at(y).at(x) = 0.0f; 
                }
                else if (waterPercentage < 0.05f) {
                    noiseValue = pow(noiseValue, flatten);
                    heightMap.at(y).at(x) = ofMap(noiseValue, 0, 1, 10, maxElevation); 
                }
                else {
                    float blendedValue = (noiseValue + euclideanDistance / maxEuclideanDistance) / 2.0f;
                    blendedValue = pow(blendedValue, flatten);
                    heightMap.at(y).at(x) = ofMap(blendedValue, 0, 1, -maxElevation, maxElevation);
                    if (heightMap.at(y).at(x) < 0.0f)
                        heightMap.at(y).at(x) = 0.0f;
                }
            }
        }
    }
}


std::vector<std::vector<float>> Fjord::getHeightMap() {
    return heightMap;
}

int Fjord::getSize() {
    return size;
}

int Fjord::getTileSize() {
    return tileSize;
}

int Fjord::getMaxElevation() {
    return maxElevation;
}