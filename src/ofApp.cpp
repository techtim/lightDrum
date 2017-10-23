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
#include "ofApp.h"

static const string LD_ENABLE_LED_MAP = "enable led mapping";

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetVerticalSync(true);
    ofBackground(50);
    ofSetLogLevel(OF_LOG_VERBOSE);

    // print input ports to console
    m_midiIn.listPorts(); // via instance

    m_midiPortNum = m_midiIn.getNumPorts()-1;
    // open port by number (you may need to change this)
    m_midiIn.openPort(m_midiPortNum);
    //midiIn.openPort("IAC Pure Data In");	// by name
    //midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port

    // don't ignore sysex, timing, & active sense messages,
    // these are ignored by default
    m_midiIn.ignoreTypes(false, false, false);
    // add ofApp as a listener
    m_midiIn.addListener(this);
    // print received messages to the console
    m_midiIn.setVerbose(true);
    
    setupGui();
}

void ofApp::setupGui() {
//    m_gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_RIGHT);
//    m_guiTheme = make_unique<LedMapper::ofxDatGuiThemeLM>();
//    m_gui->setTheme(m_guiTheme.get());
//    m_gui->setWidth(LM_GUI_WIDTH);
//    
//    m_gui->addToggle(LD_ENABLE_LED_MAP, false);
}

//--------------------------------------------------------------
void ofApp::update() {
    m_fbo.begin();

    ofClear(0,0,0);
    
    m_fbo.end();
    m_fbo.readToPixels(m_grabPixels);
//
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofSetColor(200);

    // draw the last recieved message contents to the screen
    text << "Received: " << ofxMidiMessage::getStatusString(m_midiMessage.status);
    ofDrawBitmapString(text.str(), 20, 20);
    text.str(""); // clear
    
    if (m_midiMessage.status == MIDI_NOTE_ON) {
        ofLog(OF_LOG_VERBOSE) << ofxMidiMessage::getStatusString(m_midiMessage.status)
        << " channel: " << m_midiMessage.channel << " pitch: " << m_midiMessage.pitch
        << " velocity: " << m_midiMessage.velocity << " control: " << m_midiMessage.control
        << " value: " << m_midiMessage.value;
    }

    if (m_midiMessage.status == MIDI_PROGRAM_CHANGE) {
        ofLog(OF_LOG_VERBOSE) << "MIDI_PROGRAM_CHANGE - " << "channel: " <<  m_midiMessage.channel
        << " pitch: " << m_midiMessage.pitch
        << " velocity: " << m_midiMessage.velocity << "control: " << m_midiMessage.control
        << " value: " << m_midiMessage.value;
    }
    text << "channel: " << m_midiMessage.channel;
    ofDrawBitmapString(text.str(), 20, 34);
    text.str(""); // clear
    
    text << "pitch: " << m_midiMessage.pitch;
    ofDrawBitmapString(text.str(), 20, 48);
    text.str(""); // clear
    ofRectangle(20, 58, ofMap(m_midiMessage.pitch, 0, 127, 0, ofGetWidth()-40), 20);
    
    text << "velocity: " << m_midiMessage.velocity;
    ofDrawBitmapString(text.str(), 20, 96);
    text.str(""); // clear
    ofRectangle(20, 105, ofMap(m_midiMessage.velocity, 0, 127, 0, ofGetWidth()-40), 20);
    
    text << "control: " << m_midiMessage.control;
    ofDrawBitmapString(text.str(), 20, 144);
    text.str(""); // clear
    ofRectangle(20, 154, ofMap(m_midiMessage.control, 0, 127, 0, ofGetWidth()-40), 20);
    
    text << "value: " << m_midiMessage.value;
    ofDrawBitmapString(text.str(), 20, 192);
    text.str(""); // clear
    if(m_midiMessage.status == MIDI_PITCH_BEND) {
        ofRectangle(20, 202, ofMap(m_midiMessage.value, 0, MIDI_MAX_BEND, 0, ofGetWidth()-40), 20);
    }
    else {
        ofRectangle(20, 202, ofMap(m_midiMessage.value, 0, 127, 0, ofGetWidth()-40), 20);
    }

    text << "delta: " << m_midiMessage.deltatime;
    ofDrawBitmapString(text.str(), 20, 240);
    text.str(""); // clear
}

//--------------------------------------------------------------
void ofApp::exit() {

    // clean up
    m_midiIn.closePort();
    m_midiIn.removeListener(this);
}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {

    // make a copy of the latest message
    m_midiMessage = msg;
}

bool ofApp::setMidiPort(size_t port) {
    if (port < m_midiIn.getPortList().size()) {
        m_midiPortNum = port;
        m_midiIn.openPort(port);
        return true;
    }
    return false;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

    switch(key) {
        case 'i':
            m_midiIn.listPorts();
            break;
        case OF_KEY_UP:
            ++m_midiPortNum;
            if (!setMidiPort(m_midiPortNum))
                --m_midiPortNum;
            break;
        case OF_KEY_DOWN:
            --m_midiPortNum;;
            if (!setMidiPort(m_midiPortNum))
                ++m_midiPortNum;
            break;
        case '3':
            setMidiPort(2);
            break;
        case '4':
            setMidiPort(3);
            break;
        case 's':
            m_ledCtrl->save("");
            break;
        case 'l':
            m_ledCtrl->load("");
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

void ofApp::onScrollViewEvent(ofxDatGuiScrollViewEvent e)
{
    if (e.parent->getName() == LMGUIListControllers) {
        // check if item from list selected
//        setCurrentController(ofToInt(e.target->getName()));
    }
}

void ofApp::onButtonClick(ofxDatGuiButtonEvent e)
{
    if (e.target->getName() == LD_ENABLE_LED_MAP) {
        m_ledCtrl->setSelected(dynamic_cast<ofxDatGuiToggle *>(e.target)->getChecked());
//        add(configFolderPath);
    }
}

void ofApp::onSliderEvent(ofxDatGuiSliderEvent e)
{
    if (e.target->getName() == LMGUISliderFps) {
        ofSetFrameRate(e.target->getValue());
    }
}
