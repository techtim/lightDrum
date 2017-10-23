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

struct Pad {
    ofRectangle bounds;
    ofVec2f from;
    ofVec2f to;
    int pitch;
    float value;
};

static void to_json(ofJson& j, const Pad& p) {
    j = ofJson{
        {"from", {{"x", p.from.x}, {"y", p.from.y}}},
        {"to", {{"x", p.to.x}, {"y", p.to.y}}},
        {"pitch", p.pitch}};
}

static void from_json(const ofJson& j, Pad& p) {
    p.from.x = j.at("from").at("x").get<float>();
    p.from.y = j.at("from").at("y").get<float>();
    p.to.x = j.at("to").at("x").get<float>();
    p.to.y = j.at("to").at("y").get<float>();
    p.pitch = j.at("pitch").get<int>();
}

class Drum {
    
    ofJson m_config;
    unique_ptr<ofxLedController> m_ledCtrl;
    ofPixels m_grabImage;
    map<size_t, float> m_chanEvelop;
    map<size_t, Pad> m_pads;
    map<int, size_t> m_pitchToPad;
    unique_ptr<ofxDatGui> m_gui;
    unique_ptr<ofxDatGuiTheme> m_guiTheme;

    vector<unique_ptr<Scene>> m_scenes;
    size_t m_currentScene;
public:
    Drum(const string &path) {
        m_ledCtrl = make_unique<ofxLedController>(0, "");
        loadPads(*m_ledCtrl);
        setupGui();
    }
    
    void loadPads(const ofxLedController &ledCtrl) {
        auto &chanToGrabs = ledCtrl.peekGrabObjects();
        size_t cntr_id = 1;
        m_pads.clear();
        for (auto &chan: chanToGrabs)
            for (auto &grab : chan) {
                Pad pad{grab->getBounds(), grab->getFrom(), grab->getTo(), 0, 0.f};
                m_pads[cntr_id] = move(pad);
            }

    }
    
    void setupGui() {
        m_gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_RIGHT);
        m_guiTheme = make_unique<LedMapper::ofxDatGuiThemeLM>();
        m_gui->setTheme(m_guiTheme.get());
        m_gui->setWidth(LM_GUI_WIDTH);
    
        m_gui->addToggle(LD_ENABLE_LED_MAP, false);
    }

    void onScrollViewEvent(ofxDatGuiScrollViewEvent e);
    void onButtonClick(ofxDatGuiButtonEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);
    
    void update() {
        m_ledCtrl->sendUdp(m_grabImage);
    }
    
    void draw() {
        
        for (auto &pad : m_pads) {
            
        }
    }
    
    void load(){
        m_ledCtrl->load("");
    }
};