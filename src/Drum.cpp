//
//  Drum.cpp
//  lightDrum
//
//  Created by Timofey Tavlintsev on 10/18/17.
//
//

#include "Drum.h"

/// GUI
static const string LDGUIButtonLedMap = "enable led mapping";
static const string LDGUIButtonAdd = "add scene";
static const string LDGUIListScenes = "scenes";

static const size_t s_defaultStartNote = 12;

Drum::Drum(const string &path)
: m_currentScene(0)
, m_ledCtrl(make_unique<ofxLedController>(0, ""))
, m_grabBounds(glm::vec2(0,0), 800, 800)
{
    // setupGui must call first
    setupGui();
    
    m_fbo.allocate(m_grabBounds.width, m_grabBounds.height, GL_RGB);
    m_fbo.begin();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_fbo.end();
    m_grabImage.allocate(m_grabBounds.width, m_grabBounds.height, GL_RGB);
    
    addScene();
    load();
}

void Drum::setupGui()
{
    m_gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_RIGHT);
    m_guiTheme = make_unique<LedMapper::ofxDatGuiThemeLM>();
    m_gui->setTheme(m_guiTheme.get());
    m_gui->setWidth(LM_GUI_WIDTH);
    m_gui->addHeader("Drum");
    
    auto tgl = m_gui->addToggle(LDGUIButtonLedMap, false);
    tgl->onButtonEvent(this, &Drum::onButtonClick);
    
    auto btn = m_gui->addButton(LDGUIButtonAdd);
    btn->onButtonEvent(this, &Drum::onButtonClick);

    m_listScenes = make_unique<ofxDatGuiScrollView>(LDGUIListScenes, 7);
    m_listScenes->onScrollViewEvent(this, &Drum::onScrollViewEvent);
    m_listScenes->setWidth(LM_GUI_WIDTH);
    m_listScenes->setPosition(m_gui->getPosition().x, m_gui->getPosition().y + m_gui->getHeight());
    m_listScenes->setBackgroundColor(ofColor(10));

    m_ledCtrl->setGuiPosition(m_listScenes->getX(), m_listScenes->getY() + m_listScenes->getHeight());
}

void Drum::update() {
    m_ledCtrl->sendUdp(m_grabImage);
    m_gui->update();
    m_listScenes->update();
    if (m_currentScene < m_scenes.size())
        m_scenes[m_currentScene].update();
}

void Drum::draw()
{
    m_gui->draw();
    
    m_listScenes->draw();
    
    if (m_currentScene < m_scenes.size()) {
        m_scenes[m_currentScene].draw();
        m_fbo.begin();
        ofClear(0,0,0,0.01);
        for (auto &pad : m_pads) {
            m_scenes[m_currentScene].updateAndDraw(pad.second);
        }
        m_fbo.end();
        ofSetColor(255);
        m_fbo.draw(m_grabBounds);
        m_fbo.readToPixels(m_grabImage);
    }

    m_ledCtrl->draw();
}

void Drum::onMidiMessage(ofxMidiMessage &midi)
{
    if (midi.status == MIDI_NOTE_ON) {
        if (m_pitchToPad.find(midi.pitch) == m_pitchToPad.end())
            return;
        ofLogVerbose() << "Pitch To Pad : " << midi.pitch << "->" << m_pitchToPad[midi.pitch];
        m_pads[m_pitchToPad[midi.pitch]].lastTrigTime = ofGetSystemTime();
        m_pads[m_pitchToPad[midi.pitch]].velocity = midi.velocity;
    }
    else if (midi.status == MidiStatus::MIDI_PROGRAM_CHANGE) {
    }
}

/// GUI EVENTS
void Drum::onScrollViewEvent(ofxDatGuiScrollViewEvent e)
{
    if (e.parent->getName() == LDGUIListScenes) {
        selectScene(e.index);
        // check if item from list selected
        //        setCurrentController(ofToInt(e.target->getName()));
    }
}

void Drum::onButtonClick(ofxDatGuiButtonEvent e)
{
    if (e.target->getName() == LDGUIButtonLedMap) {
        bool bEnable = dynamic_cast<ofxDatGuiToggle *>(e.target)->getChecked();
        ofLogVerbose() << LDGUIButtonLedMap << " clicked => " << bEnable;
        m_ledCtrl->setSelected(bEnable);
        //        add(configFolderPath);
    }
    if (e.target->getName() == LDGUIButtonAdd) {
        addScene();
    }
}

void Drum::addScene() {
    m_scenes.emplace_back(Scene());
    m_listScenes->add(ofToString(m_scenes.size()));
}

void Drum::selectScene(size_t num) {
    m_listScenes->get(m_currentScene)->setBackgroundColor(0);
    m_scenes[m_currentScene].setEnable(false);
    m_currentScene = num % m_scenes.size();
    m_listScenes->get(m_currentScene)->setBackgroundColor(50);
    m_scenes[m_currentScene].setEnable(true);
    ofLogVerbose() << "Select m_currentScene=" << m_currentScene;
}
void Drum::onSliderEvent(ofxDatGuiSliderEvent e)
{
    if (e.target->getName() == LMGUISliderFps) {
        ofSetFrameRate(e.target->getValue());
    }
}

/// SAVE & LOAD
void Drum::load()
{
    m_ledCtrl->load("");
    m_grabBounds = m_ledCtrl->peekBounds();
    m_grabImage.allocate(m_grabBounds.width, m_grabBounds.height, GL_RGB);
    m_fbo.allocate(m_grabBounds.width, m_grabBounds.height, GL_RGB);
    m_config = ofLoadJson("drum_conf.json");
    loadPads(*m_ledCtrl);
//    m_pads = m_config.at("pads").get<map<size_t, Pad>>();

    //    m_config["pads"] = m_pads;
}

void Drum::loadPads(const ofxLedController &ledCtrl)
{
    auto &chanToGrabs = ledCtrl.peekGrabObjects();
    size_t cntr_id = 0;
    m_pads.clear();
    for (auto &chan : chanToGrabs)
        for (auto &grab : chan) {
            Pad pad{ grab->getBounds(), grab->getFrom(), grab->getTo(), static_cast<int>(s_defaultStartNote+cntr_id), 0.f, 0 };
            m_pitchToPad[pad.pitch] = cntr_id;
            m_pads[cntr_id] = move(pad);
            ofLog(OF_LOG_VERBOSE) << "Pad load id=" << cntr_id << " bounds=" << pad.bounds;
            cntr_id++;
        }
}

void Drum::save()
{
    m_ledCtrl->save("");

//    m_config["pads"] = m_pads;
//    ofstream jsonFile(ofToDataPath("drum_conf.json"));
//    jsonFile << m_config.dump(4);
//    jsonFile.close();
}