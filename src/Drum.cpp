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
static const string LDGUIButtonLedLoad = "load from map";
static const string LDGUIButtonAdd = "add scene";
static const string LDGUIListScenes = "scenes";

static const size_t s_defaultStartNote = 12;
static const uint64_t MILLISEC_IN_FRAME = 1000.0 / 20.0; // 30 FPS

Drum::Drum(const string &path)
    : m_ledCtrl(make_unique<ofxLedController>(0, ""))
    , m_currentScene(0)
    , m_grabBounds(0, 0, 800, 800)
    , m_lastFrameTime(0)
{
    // setupGui must call first
    setupGui();

    /// RPI specific RGBA textures, works correct only this
    m_fbo.allocate(m_grabBounds.width, m_grabBounds.height, GL_RGBA);
    m_fbo.begin();
    ofClear(0, 0, 0, 0);
    m_fbo.end();
    m_grabImage.allocate(m_grabBounds.width, m_grabBounds.height, OF_IMAGE_COLOR_ALPHA);

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

    auto btn = m_gui->addButton(LDGUIButtonLedLoad);
    btn->onButtonEvent(this, &Drum::onButtonClick);

    btn = m_gui->addButton(LDGUIButtonAdd);
    btn->onButtonEvent(this, &Drum::onButtonClick);

    m_listScenes = make_unique<ofxDatGuiScrollView>(LDGUIListScenes, 7);
    m_listScenes->onScrollViewEvent(this, &Drum::onScrollViewEvent);
    m_listScenes->setWidth(LM_GUI_WIDTH);
    m_listScenes->setPosition(m_gui->getPosition().x, m_gui->getPosition().y + m_gui->getHeight());
    m_listScenes->setBackgroundColor(ofColor(10));

    m_ledCtrl->setGuiPosition(m_listScenes->getX(),
                              m_listScenes->getY() + m_listScenes->getHeight());
}

void Drum::update()
{
    auto now = ofGetSystemTime();
    if (now - m_lastFrameTime > MILLISEC_IN_FRAME) {
        m_ledCtrl->sendUdp(m_grabImage);
        m_lastFrameTime = now;
    }
    m_gui->update();
    m_listScenes->update();
    if (m_currentScene < m_scenes.size())
        m_scenes[m_currentScene].update();
}

void Drum::draw()
{


    if (m_currentScene < m_scenes.size()) {
        m_fbo.begin();
        glClearColor(0, 0, 0, 100);
        glClear(GL_COLOR_BUFFER_BIT);
        //        ofDrawRectangle(m_grabBounds);
        for (auto &pad : m_pads) {
            m_scenes[m_currentScene].updateAndDraw(pad);
        }
        m_fbo.end();
        ofSetColor(255, 255, 255, 255);
        m_fbo.draw(m_grabBounds);
        m_fbo.readToPixels(m_grabImage);
        m_scenes[m_currentScene].draw();
    }
    ofSetColor(255, 255, 255, 255);
    ofNoFill();
    ofDrawRectangle(m_grabBounds);
    ofFill();

    m_ledCtrl->draw();
    m_gui->draw();
    m_listScenes->draw();
}

void Drum::onMidiMessage(ofxMidiMessage &midi)
{
    ofLog(OF_LOG_VERBOSE) << ofxMidiMessage::getStatusString(midi.status)
                          << " channel: " << midi.channel << " pitch: " << midi.pitch
                          << " velocity: " << midi.velocity << " control: " << midi.control
                          << " value: " << midi.value;

    if (midi.status == MIDI_NOTE_ON) {
        if (midi.velocity == 0 || (m_pitchToPad.find(midi.pitch) == m_pitchToPad.end()
                                   || m_pitchToPad[midi.pitch] >= m_pads.size()))
            return;
        ofLogVerbose() << "Pitch To Pad : " << midi.pitch << "->" << m_pitchToPad[midi.pitch];
        m_pads[m_pitchToPad[midi.pitch]].lastTrigTime = ofGetSystemTime();
        m_pads[m_pitchToPad[midi.pitch]].velocity = midi.velocity;
    }
    else if (midi.status == MidiStatus::MIDI_PROGRAM_CHANGE) {
        ofLog(OF_LOG_VERBOSE) << "MIDI_PROGRAM_CHANGE - "
                              << "channel: " << midi.channel << " pitch: " << midi.pitch
                              << " velocity: " << midi.velocity << "control: " << midi.control
                              << " value: " << midi.value;
    }
}

/// GUI EVENTS
void Drum::onScrollViewEvent(ofxDatGuiScrollViewEvent e)
{
    if (e.parent->getName() == LDGUIListScenes) {
        selectScene(e.index);
    }
}

void Drum::onButtonClick(ofxDatGuiButtonEvent e)
{
    if (e.target->getName() == LDGUIButtonLedMap) {
        bool bEnable = dynamic_cast<ofxDatGuiToggle *>(e.target)->getChecked();
        ofLogVerbose() << LDGUIButtonLedMap << " clicked => " << bEnable;
        m_ledCtrl->setSelected(bEnable);
    }
    if (e.target->getName() == LDGUIButtonLedLoad) {
        loadPads(*m_ledCtrl);
    }

    if (e.target->getName() == LDGUIButtonAdd) {
        addScene();
    }
}

void Drum::addScene()
{
    m_scenes.emplace_back(Scene(m_scenes.size() + 1));
    m_listScenes->add(ofToString(m_scenes.size()));
}

void Drum::selectScene(size_t num)
{
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
void Drum::loadPads(const ofxLedController &ledCtrl)
{
    auto &chanToGrabs = ledCtrl.peekGrabObjects();
    size_t cntr_id = 0;
    m_pads.clear();
    for (auto &chan : chanToGrabs)
        for (auto &grab : chan) {
            auto pad = Pad(grab->getBounds(), grab->getFrom(), grab->getTo(),
                           static_cast<int>(s_defaultStartNote + cntr_id), 0.f, 0);
            m_pitchToPad[pad.pitch] = cntr_id;
            ofLog(OF_LOG_VERBOSE) << "Pad load id=" << cntr_id << " bounds=" << pad.bounds;
            m_pads.emplace_back(move(pad));
            cntr_id++;
        }
}

void Drum::load()
{
    m_ledCtrl->load("");
    m_ledCtrl->showGui(true);

    m_grabBounds = m_ledCtrl->peekBounds();
    m_grabImage.resize(m_grabBounds.width, m_grabBounds.height);
    m_fbo.allocate(m_grabBounds.width, m_grabBounds.height, GL_RGBA);

    m_config = ofLoadJson("drum_conf.json");

    try {
        m_pads = m_config.at("pads").get<vector<Pad>>();
        for (size_t cntr = 0; cntr < m_pads.size(); ++cntr) {
            ofLog(OF_LOG_VERBOSE) << "Pad load id=" << cntr << " pitch=" << m_pads[cntr].pitch
                                  << " bounds=" << m_pads[cntr].bounds
                                  << " from=" << m_pads[cntr].from << " to=" << m_pads[cntr].to;
            m_pitchToPad[m_pads[cntr].pitch] = cntr;
        }
    }
    catch (std::logic_error &err) {
        ofLogError() << "Pads load Failed: " << err.what();
        loadPads(*m_ledCtrl);
    }

    //    try {
    //        m_scenes = m_config.at("scenes").get<vector<Scene>>();
    //    }
    //    catch(std::logic_error &err) {
    //        ofLogError() << "Scenes load Failed: " << err.what();
    //    }
    ofLogVerbose() << "Load Drum finished: m_grabBounds=" << m_grabBounds
                   << " num pads=" << m_pads.size();
    //    m_config["pads"] = m_pads;
}

void Drum::save()
{
    m_ledCtrl->save("");

    ofJson pads_array;
    for (auto &pad : m_pads) {
        pads_array.emplace_back(pad);
    }
    m_config["pads"] = pads_array;
    m_config["scenes"] = m_scenes;
    ofstream jsonFile(ofToDataPath("drum_conf.json"));
    jsonFile << m_config.dump(4);
    jsonFile.close();
}