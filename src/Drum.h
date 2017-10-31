//
//  Drum.h
//  lightDrum
//
//  Created by Timofey Tavlintsev on 10/18/17.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLedController.h"
#include "Scene.h"
#include "ofxMidiMessage.h"

using MidiHandler = function<void(const ofxMidiMessage&)>;

class Drum {
    
    ofJson m_config;
    unique_ptr<ofxLedController> m_ledCtrl;
    ofPixels m_grabImage;
    map<size_t, float> m_chanEvelop;
    map<size_t, Pad> m_pads;
    map<int, size_t> m_pitchToPad;
    unique_ptr<ofxDatGui> m_gui;
    unique_ptr<ofxDatGuiScrollView> m_listScenes;
    unique_ptr<ofxDatGuiTheme> m_guiTheme;
    ofFbo m_fbo;
    vector<Scene> m_scenes;
    size_t m_currentScene;
    ofRectangle m_grabBounds;
    MidiHandler m_midiHandler;
    
public:
    Drum(const string &path = "");
    
    void addScene();
    void selectScene(size_t num);
    void loadPads(const ofxLedController &ledCtrl);
    void setupGui();
    
    void onMidiMessage(ofxMidiMessage& eventArgs);
    
    void onScrollViewEvent(ofxDatGuiScrollViewEvent e);
    void onButtonClick(ofxDatGuiButtonEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);
    
    void update();
    void draw();
    
    void load();
    void save();
};