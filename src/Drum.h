//
//  Drum.h
//  lightDrum
//
//  Created by Timofey Tavlintsev on 10/18/17.
//
//

#pragma once

#include "ofMain.h"
#include "ofxMidiMessage.h"

//#define LED_MAPPER_NO_GUI 1
#include "Pad.h"
#include "Scene.h"
#include "ofxLedController.h"

using MidiHandler = function<void(const ofxMidiMessage &)>;

class Drum {

    ofJson m_config;
    unique_ptr<ofxLedController> m_ledCtrl;
    ofPixels m_grabImage;
    ofFbo m_fbo;

    vector<Pad> m_pads;
    map<int, size_t> m_pitchToPad;
    vector<Scene> m_scenes;
    size_t m_currentScene, m_nextScene;
    ofRectangle m_grabBounds;
    uint64_t m_lastFrameTime;
    int m_fadeAmnt;
    int m_midiChannel, m_midiDevice;
    uint64_t m_lockedByRemoteTime;
    bool m_bNeedLoad;
    unique_ptr<ofxDatGui> m_gui, m_guiScene;
    unique_ptr<ofxDatGuiScrollView> m_listScenes;

    ofxUDPManager m_configSender, m_configReceiver;

    void loadFromJson(const ofJson &json);
    void realLoad();

public:
    Drum(const string &path = "");

    void addScene();
    void selectScene(size_t num);
    void loadPads(const ofxLedController &ledCtrl);
    void setupGui();

    int getMidiChannel() const { return m_midiChannel; }
    int getMidiDevice() const { return m_midiDevice; }
    void onMidiMessage(const ofxMidiMessage &eventArgs);

    void onScrollViewEvent(ofxDatGuiScrollViewEvent e);
    void onToggleClick(ofxDatGuiToggleEvent e);
    void onButtonClick(ofxDatGuiButtonEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);

    void reset();

    void update();
    void draw();

    void load();
    void save(size_t midiDevice = 0);

    void sendBlock();
    void sendConfig();
    void receiveConfigOrBlock();
};

static const int LD_COLOR_GREEN_LIGHT = 0x6BE6B4;

class ofxDatGuiThemeLD : public ofxDatGuiTheme {
public:
    ofxDatGuiThemeLD()
    {
        stripe.visible = false;
        color.label = hex(0x9C9DA1);
        color.icons = hex(0x9C9DA1);
        color.background = hex(0x28292E);
        color.guiBackground = hex(0x1E1F24);
        color.inputAreaBackground = hex(0x42424A);
        color.slider.fill = hex(LD_COLOR_GREEN_LIGHT); // 107 230 180
        color.slider.text = hex(0x9C9DA1);
        color.textInput.text = hex(0x9C9DA1);
        color.textInput.highlight = hex(0x28292E);
        color.colorPicker.border = hex(0xEEEEEE);
        color.textInput.backgroundOnActive = hex(0x1D1E22);
        color.backgroundOnMouseOver = hex(0x42424A);
        color.backgroundOnMouseDown = hex(0x1D1E22);
        color.matrix.hover.button = hex(0x9C9DA1);
        color.graph.fills = hex(0x9C9DA1);
        stripe.button = hex(0x64ffda);
        stripe.toggle = hex(0x64ffda);
        init();
    }
};
