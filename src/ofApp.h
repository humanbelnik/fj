#pragma once
#pragma execution_character_set("utf-8")
#include "ofMain.h"
#include "ofxGui.h"
#include "../render.h"
#include "../generator.h"
#include "../noise.h"


class ofApp : public ofBaseApp {
private:
    std::unique_ptr<RenderEngine> renderEngine;
    ofxPanel gui;

    ofxIntSlider tileSizeSlider;
    ofxIntSlider maxElevationSlider;
    ofxIntSlider scaleFactorSlider;
    ofxFloatSlider lakeSizeSlider;
    ofxButton regenerateButton;
    ofxButton changeTexture;
    ofxToggle isLakeToggle;



    ofxLabel tileLabel;
    ofxLabel octaveLabel;
    ofxLabel heightLabel;
    ofxLabel isLakeLabel;
    ofxLabel lakeSizeLabel;
    ofxLabel updateLabel;
    ofxLabel other; 




    bool needsRedraw = true;

    bool _regen = true;
    int octave = 8;
    int seed = 1;
    int maxElevation = 3000;
    int tileSize = 10;
    bool isLake = false;
    float waterPercentage = 0.5f;


    void setupGUI();
    void updateLandscapeSettings();

public:
    void onTileSizeChanged(int& value);
    void onMaxElevationChanged(int& value);
    void onScaleFactorChanged(int& value);
    void onRegeneratePressed();
    void onLakeSizeChanged(float& value);
    void onTexturePressed();
    void onIsLakeChanged(bool& value);
    void setup();
    void draw();

    void keyPressed(int key);
};