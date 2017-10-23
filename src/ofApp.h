/*
 * Copyright (c) 2013 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/danomatika/ofxMidi for documentation
 *
 */
#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxDatGui.h"
#include "ofxLedController.h"

class ofApp : public ofBaseApp, public ofxMidiListener {
    
public:
    
    void setup();
    void setupGui();
    
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    
//    void mouseMoved(int x, int y );
//    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
//    void mouseReleased();
    
    void onScrollViewEvent(ofxDatGuiScrollViewEvent e);
    void onButtonClick(ofxDatGuiButtonEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);
    
    void newMidiMessage(ofxMidiMessage& eventArgs);
    bool setMidiPort(size_t port);
    
    stringstream text;
    
    size_t m_midiPortNum;
    ofxMidiIn m_midiIn;
    ofxMidiMessage m_midiMessage;

    ofFbo m_fbo;
    
    unique_ptr<ofxDatGui> m_gui;
    unique_ptr<ofxDatGuiTheme> m_guiTheme;
    
    ofJson m_config;
    unique_ptr<ofxLedController> m_ledCtrl;
    ofPixels m_grabPixels;
};
