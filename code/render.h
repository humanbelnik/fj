#pragma once

#define _USE_MATH_DEFINES
#include "ofMain.h"
#include "ofVec3f.h"
#include "fjord.h"
#include "generator.h"
#include "noise.h"
#include <vector>
#include <cmath>
#include <math.h>
#include <functional>

class RenderEngine {
private:
	std::unique_ptr<Fjord> fjord;

	glm::mat4 modelMatrix;
	glm::vec3 translation;
	float rotationAngle;
	float cameraDistance;
	glm::vec3 lightPos;
	float scaleFactor;
	int mapType = 1;

	void rasterizeTriangle(const glm::vec4 vertices[3], std::vector<std::vector<float>>& zBuffer, int i, int j, float elev, glm::vec3 norm);
	glm::mat4 setupProjection();
	ofColor calculateColor(float height, float lightIntensity);
	ofColor interpolateColor(float elev, float l, float h, ofColor lc, ofColor hc);

public:
	RenderEngine(std::unique_ptr<HeightGenerator_Creator> generator_creator, std::function<float(float, float)>);
	void render();
	void update(
		bool _regen = true, int octave = 8, int seed = 0,
		int maxElevation = 3000, int tileSize = 20, bool isLake = false, float waterPercentage = 0.5
	);
	void changeMapType();
	void rotate(bool clockwise);
	void zoom(bool zoomIn);
};