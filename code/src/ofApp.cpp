#include "ofApp.h"
void ofApp::setup() {
    auto generatorCreator = std::make_unique<OctaveGenerator_Creator>();
    renderEngine = std::make_unique<RenderEngine>(std::move(generatorCreator), noise);

    ofSetWindowTitle("Landscape Visualizer");
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    _regen = true;
    setupGUI();
}

void ofApp::setupGUI() {
    setlocale(LC_ALL, "ru-RU");
    ofxGuiSetBackgroundColor(ofColor(0, 0, 0));
    ofxGuiSetFont("Lunasima-Regular.ttf", 18);
    ofxGuiSetDefaultWidth(550);
    gui.setup();

    gui.add(tileLabel.setup("", u8"Use slider to define surface step", 550, 50));
    gui.add(tileSizeSlider.setup("", 20, 1, 100, 400, 50));
    tileSizeSlider.addListener(this, &ofApp::onTileSizeChanged);

    gui.add(heightLabel.setup("", "Use slider to define steepness", 600, 50));
    gui.add(maxElevationSlider.setup("", 3000, 0, 6000, 400, 50));
    maxElevationSlider.addListener(this, &ofApp::onMaxElevationChanged);

    gui.add(octaveLabel.setup("", "Use slider to define smoothiness", 600, 50));
    gui.add(scaleFactorSlider.setup("", 8, 1, 10, 400, 50));
    scaleFactorSlider.addListener(this, &ofApp::onScaleFactorChanged);

    gui.add(regenerateButton.setup("Press to build new landscape", 400, 50));
    regenerateButton.addListener(this, &ofApp::onRegeneratePressed);

    gui.add(changeTexture.setup("Press to change textures", 400, 50));
    changeTexture.addListener(this, &ofApp::onTexturePressed);

    gui.add(isLakeToggle.setup("Toggle to enter lake mode", false , 400, 50));
    isLakeToggle.addListener(this, &ofApp::onIsLakeChanged);

    gui.add(lakeSizeLabel.setup("", "Use slider to define lake size", 600, 50));
    gui.add(lakeSizeSlider.setup("", 0.5f, 0.0f, 1.0f, 400, 50));
    lakeSizeSlider.addListener(this, &ofApp::onLakeSizeChanged);
    gui.add(other.setup("", "Use keyboard arrows to zoom and rotate", 600, 50));


}

void ofApp::onIsLakeChanged(bool& value) {
    isLake = value;
    _regen = true;
    seed = ofRandom(1, 10000);
    updateLandscapeSettings();
    needsRedraw = true;
}

void ofApp::onTexturePressed() {
    keyPressed(116);
}


void ofApp::onLakeSizeChanged(float& value) {
    if (!isLake) {
        return;
    }
    waterPercentage = value;
    _regen = true;
    seed = ofRandom(1, 10000);
    updateLandscapeSettings();
    needsRedraw = true;
}

void ofApp::onRegeneratePressed() {
    seed = ofRandom(1, 10000);
    _regen = true;
    updateLandscapeSettings();
    needsRedraw = true;
}

void ofApp::onTileSizeChanged(int& value) {
    _regen = false;
    tileSize = value;
    needsRedraw = true;

}
void ofApp::onMaxElevationChanged(int& value) {
    _regen = false;
    maxElevation = value;
    needsRedraw = true;
}
void ofApp::onScaleFactorChanged(int& value) {
    _regen = false;
    octave = value;
    needsRedraw = true;
}

void ofApp::updateLandscapeSettings() {
    if (needsRedraw) {
        needsRedraw = false;
    }
    renderEngine->update(
        _regen,
        octave,
        seed,
        maxElevation,
        tileSize,
        isLake,
        waterPercentage
    );
}

void ofApp::draw() {
    ofBackground(50, 50, 50);
    if (needsRedraw) {
        renderEngine->update(
            _regen,
            octave,
            seed,
            maxElevation,
            tileSize,
            isLake,
            waterPercentage
        );
        needsRedraw = false;
    }
    renderEngine->render();
    gui.draw();
}

void ofApp::keyPressed(int key) {
    switch (key) {
    case 't':
        renderEngine->changeMapType();
        break;
    case OF_KEY_UP:
        renderEngine->zoom(true);
        break;
    case OF_KEY_DOWN:
        renderEngine->zoom(false);
        break;
    case OF_KEY_LEFT:
        renderEngine->rotate(false);
        break;
    case OF_KEY_RIGHT:
        renderEngine->rotate(true);
        break;
    }
}