#pragma once

#include "ofMain.h"
#include "ofxMidi.h"

#include "Common.h"

#include "Drum.h"

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

    void newMidiMessage(ofxMidiMessage &eventArgs);
    bool setMidiPort(size_t port);

    ofxMidiMessage makeFakeMidi(int pitch, int channel);

    stringstream text;

    string m_midiPortName;
    size_t m_midiPortNum;
    ofxMidiIn m_midiIn;
    ofxMidiMessage m_midiMessage;

    Drum m_drum;

    ofJson m_config;
};
