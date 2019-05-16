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
static const string LDGUIButtonLedSetup = "setup from map";
static const string LDGUIButtonLoad = "Load";
static const string LDGUIButtonSave = "Save";
static const string LDGUIButtonSendConfig = "SEND config to Drum";
static const string LDGUIButtonAdd = "Add scene";
static const string LDGUIListScenes = "scenes";
static const int LDGUIListSize = 4;
static const int LD_GUI_WIDTH = 200;
static const size_t s_defaultStartNote = 12;
static const uint64_t MILLISEC_IN_FRAME = 1000.0 / 25.0; // 25 FPS sending speed to LEDs
static const uint64_t s_lockTime = 1000;

unsigned short s_configInPort = 13131;

using namespace LedMapper;

Drum::Drum(const string &path)
    : m_currentScene(0)
    , m_grabBounds(0, 0, 800, 800)
    , m_lastFrameTime(0)
    , m_fadeAmnt(40)
    , m_midiChannel(0)
    , m_lockedByRemoteTime(0)
    , m_bNeedLoad(true)
{
    // setupGui must call first
    setupGui();
    realLoad();

    m_configReceiver.Create();
    m_configReceiver.Bind(s_configInPort);
    m_configReceiver.SetNonBlocking(true);

#if !defined(TARGET_RASPBERRY_PI)
    m_configSender.Create();
    m_configSender.Connect(m_ledCtrl->getIP().c_str(), s_configInPort);
    m_configSender.SetSendBufferSize(4096 * 3);
    m_configSender.SetNonBlocking(true);
#endif
}

void Drum::setupGui()
{
    m_guiLedCtrl = GenerateOutputGui();
    m_guiLedCtrl->setPosition(ofGetWidth() - m_guiLedCtrl->getWidth(), ofGetHeight()/2);
    m_gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_RIGHT);
    m_guiScene = Scene::GenerateGui();
    m_guiTheme = make_unique<ofxDatGuiThemeLD>();
    m_gui->setTheme(m_guiTheme.get(), true);
    m_gui->setWidth(LD_GUI_WIDTH);
    m_gui->addHeader("Drum");

    auto slider = m_gui->addSlider("MIDI chan", 1, 16);
    slider->bind(m_midiChannel);
    slider->setPrecision(0);

    slider = m_gui->addSlider("MIDI device", 0, 16);
    slider->bind(m_midiDevice);
    slider->setPrecision(0);

    m_gui->addToggle(LDGUIButtonLedMap, false)->onToggleEvent([this](ofxDatGuiToggleEvent e) {
        m_ledCtrl->setSelected(e.checked);
    });

    m_gui->addButton(LDGUIButtonLedSetup)->onButtonEvent([this](ofxDatGuiButtonEvent e) {
        this->loadPads(*m_ledCtrl);
    });

    m_gui->addButton(LDGUIButtonSendConfig)->onButtonEvent([this](ofxDatGuiButtonEvent e) {
        this->sendConfig();
    });

    m_gui->addButton(LDGUIButtonLoad)->onButtonEvent([this](ofxDatGuiButtonEvent e) {
        this->load();
    });

    m_gui->addButton(LDGUIButtonSave)->onButtonEvent([this](ofxDatGuiButtonEvent e) {
        this->save();
    });

    m_gui->addButton(LDGUIButtonAdd)->onButtonEvent([this](ofxDatGuiButtonEvent e) {
        this->addScene();
    });

    m_listScenes = new ofxDatGuiScrollView(LDGUIListScenes, LDGUIListSize);
    m_listScenes->onScrollViewEvent([this](ofxDatGuiScrollViewEvent e) { selectScene(e.index); });
    m_listScenes->setTheme(m_guiTheme.get());
    m_listScenes->setWidth(LD_GUI_WIDTH);
    m_listScenes->setPosition(m_gui->getPosition().x, m_gui->getPosition().y + m_gui->getHeight());
    m_listScenes->setBackgroundColor(ofColor(10));

    m_gui->addFolder("Scenes")->attachItem(m_listScenes);
}

void Drum::update()
{
#if !defined(TARGET_RASPBERRY_PI)
    sendBlock();
#elif
    receiveConfigOrBlock();
#endif

    /// if not blocked by remote app send data to LEDs
    auto now = ofGetSystemTimeMillis();
    if (now - m_lockedByRemoteTime > s_lockTime) {
        m_ledCtrl->send(m_fbo.getTexture());
        m_lastFrameTime = now;
    }

    if (m_nextScene != m_currentScene)
        selectScene(m_nextScene);

    if (m_ledCtrl->isSelected())
        m_guiLedCtrl->update();
    else
        m_guiScene->update();

    m_listScenes->update();

    if (!m_scenes.empty())
        m_scenes[m_currentScene].update();
}

void Drum::draw()
{
    realLoad();
    ofEnableAlphaBlending();

    /// draw pads state with current scene
    if (!m_scenes.empty() && m_currentScene < m_scenes.size()) {
        m_fbo.begin();

        glClearColor(0, 0, 0, 255);
        glClear(GL_COLOR_BUFFER_BIT);
        ofSetColor(255, 255, 255, 255);
        for (auto &pad : m_pads) {
            m_scenes[m_currentScene].draw(pad);
        }

        m_fbo.end();
        ofSetColor(255, 255, 255, 255);
        m_fbo.draw(m_grabBounds);
    }

    ofSetColor(255, 255, 255, 255);
    ofNoFill();
    ofDrawRectangle(m_grabBounds);
    ofFill();

    m_gui->setPosition(ofGetWidth() - LM_GUI_WIDTH, 0);
    //    m_gui->draw();

    if (m_ledCtrl->isSelected()) {
        m_ledCtrl->draw();
        m_guiLedCtrl->draw();
    }
    else {
        m_guiScene->draw();
    }
}

void Drum::onMidiMessage(const ofxMidiMessage &midi)
{
    ofLog(OF_LOG_VERBOSE) << ofxMidiMessage::getStatusString(midi.status)
                          << " channel: " << midi.channel << " pitch: " << midi.pitch
                          << " velocity: " << midi.velocity << " control: " << midi.control
                          << " value: " << midi.value;

    /// m_midiChannel == 0 : listen all channels else check for selected
    if (m_midiChannel && midi.channel != m_midiChannel)
        return;

    if (midi.status == MIDI_NOTE_ON) {
        if (midi.velocity == 0
            || (m_pitchToPad.find(midi.pitch) == m_pitchToPad.end()
                   || m_pitchToPad[midi.pitch] >= m_pads.size()))
            return;

        m_pads[m_pitchToPad[midi.pitch]].lastTrigTime = ofGetSystemTime();
        m_pads[m_pitchToPad[midi.pitch]].velocity = ofMap(midi.velocity, 0.0, 127.0, 0.0f, 1.0f);
        ofLogVerbose() << "Pitch To Pad : " << midi.pitch << "->" << m_pitchToPad[midi.pitch]
                       << " velocity: " << midi.velocity << " -> "
                       << m_pads[m_pitchToPad[midi.pitch]].velocity;
    }
    else if (midi.status == MidiStatus::MIDI_PROGRAM_CHANGE) {
        ofLogWarning() << "MIDI_PROGRAM_CHANGE - "
                       << "channel: " << midi.channel << " pitch: " << midi.pitch
                       << " velocity: " << midi.velocity << "control: " << midi.control
                       << " value: " << midi.value;
        m_nextScene = midi.value;
    }
}

void Drum::addScene()
{
    m_scenes.emplace_back(Scene(m_scenes.size() + 1));
    m_listScenes->add(ofToString(m_scenes.size()));
}

void Drum::selectScene(size_t num)
{
    if (m_currentScene < m_listScenes->children.size())
        m_listScenes->get(m_currentScene)->setBackgroundColor(10);

    m_currentScene = num % m_scenes.size();
    m_nextScene = m_currentScene;

    if (m_currentScene < m_listScenes->children.size())
        m_listScenes->get(m_currentScene)->setBackgroundColor(50);

    m_scenes[m_currentScene].bindGui(m_guiScene.get());
    ofLogVerbose() << "Select m_currentScene=" << m_currentScene;
}

/// SAVE & LOAD
void Drum::loadPads(const LedMapper::ofxLedController &ledCtrl)
{
    auto &chanToGrabs = ledCtrl.peekGrabObjects();
    size_t cntr_id = 0;
    m_pads.clear();
    for (auto &chan : chanToGrabs)
        for (auto &grab : chan) {
            auto pad = Pad(grab->getBounds(), grab->getFrom(), grab->getTo(),
                s_defaultStartNote + cntr_id, 0.f, 0);
            m_pitchToPad[pad.pitch] = cntr_id;
            ofLog(OF_LOG_VERBOSE) << "Pad load id=" << cntr_id << " bounds=" << pad.bounds;
            m_pads.emplace_back(move(pad));
            cntr_id++;
        }
}

void Drum::reloadShader() {
    if (m_scenes.empty())
        return;
    
    m_scenes[m_currentScene].reloadShader();
}

void Drum::load() { m_bNeedLoad = true; }

void Drum::realLoad()
{
    if (!m_bNeedLoad)
        return;

    if (m_ledCtrl == nullptr)
        m_ledCtrl = make_unique<LedMapper::ofxLedController>(0, "");

    m_ledCtrl->load("");
    m_ledCtrl->setSelected(false);
    m_ledCtrl->bindGui(m_guiLedCtrl.get());

#if !defined(TARGET_RASPBERRY_PI)
    m_configSender.Connect(m_ledCtrl->getIP().c_str(), s_configInPort);
#endif

    /// update points and grab bounds
    m_ledCtrl->markDirtyGrabPoints();
    m_ledCtrl->updateGrabPoints();
    m_grabBounds = (m_ledCtrl != nullptr ? m_ledCtrl->peekBounds()
                                         : ofRectangle(0, 0, ofGetWidth(), ofGetHeight()));
    /// RPI specific RGBA textures, works correct only with 4 channel tex
    m_fbo.allocate(m_grabBounds.width, m_grabBounds.height, GL_RGBA);
    m_fbo.begin();
    ofClear(255, 255, 255, 0);
    m_fbo.end();
    
    /// Update shaders list for Scenes
    // Scene::loadShaders();
    
    loadFromJson(ofLoadJson("drum_conf.json"));
    
    m_listScenes->clear();
    for (size_t i = 0; i < m_scenes.size(); ++i)
        m_listScenes->add(ofToString(i + 1));

    if (m_scenes.empty())
        addScene();

    updateScenes();

    ofLogVerbose() << "Load Drum finished: m_grabBounds=" << m_grabBounds
                   << " num pads=" << m_pads.size() << " num scenes=" << m_scenes.size();
    m_config["pads"] = m_pads;
    m_bNeedLoad = false;
}

void Drum::updateScenes()
{
    m_listScenes->clear();

    for (size_t i = 0; i < m_scenes.size(); ++i)
        m_listScenes->add(ofToString(i + 1));

    if (m_scenes.empty())
        addScene();

    selectScene(m_currentScene);
}

void Drum::loadFromJson(const ofJson &json)
{

    /// 10 is the default drum channel according to general midi
    m_midiChannel = json.count("midiChannel") ? json.at("midiChannel").get<int>() : 10;

    m_midiDevice = json.count("midiDevice") ? json.at("midiDevice").get<int>() : 0;

    if (json.count("pads")) {
        try {
            m_pads = json.at("pads").get<vector<Pad>>();
            for (size_t cntr = 0; cntr < m_pads.size(); ++cntr) {
                ofLog(OF_LOG_VERBOSE) << "Pad load id=" << cntr << " pitch=" << m_pads[cntr].pitch
                                      << " bounds=" << m_pads[cntr].bounds
                                      << " from=" << m_pads[cntr].from << " to=" << m_pads[cntr].to;
                m_pitchToPad[m_pads[cntr].pitch] = cntr;
            }
        }
        catch (std::exception &err) {
            ofLogError() << "Pads load Failed: " << err.what();
            loadPads(*m_ledCtrl);
        }
    }

    if (json.count("scenes")) {
        try {
            m_scenes.clear();
            m_scenes = json.at("scenes").get<vector<Scene>>();
            ofLogVerbose() << m_scenes.size() << " scenes load";
        }
        catch (std::exception &err) {
            ofLogError() << "Scenes load Failed: " << err.what();
        }
    }
}

void Drum::save(size_t midiDevice)
{
    m_ledCtrl->save("");

    m_midiDevice = midiDevice;

    m_config.clear();

    ofJson pads_array;
    for (auto &pad : m_pads) {
        pads_array.emplace_back(pad);
    }
    m_config["pads"] = pads_array;
    m_config["scenes"] = m_scenes;
    m_config["midiChannel"] = m_midiChannel;
    m_config["midiDevice"] = m_midiDevice;
    string path = ofToDataPath("drum_conf.json");
    ofstream jsonFile(path);
    jsonFile << m_config.dump(4);
    jsonFile.close();
}

constexpr char flag = 1;
void Drum::sendBlock()
{
    if (m_ledCtrl->isStatusOk() && m_ledCtrl->isSending()) {
        m_configSender.Send(&flag, 1);
    }
}

void Drum::sendConfig()
{
#if !defined(TARGET_RASPBERRY_PI)
    m_configSender.Create();
    m_configSender.Connect(m_ledCtrl->getIP().c_str(), s_configInPort);
    m_configSender.SetSendBufferSize(4096 * 3);
    m_configSender.SetNonBlocking(true);
    save();
    m_config["midiDevice"] = 1;
    string conf = m_config.dump();
    m_configSender.Send(conf.c_str(), conf.size());
#endif
}

static char s_udpMessage[20000];

void Drum::receiveConfigOrBlock()
{
    int size = m_configReceiver.Receive(s_udpMessage, s_configInPort);
    if (size == 0)
        return;

    if (size == 1) {
        m_lockedByRemoteTime = ofGetSystemTime();
        return;
    }

    try {
        istringstream in(string(s_udpMessage, size));
        ofJson json;
        in >> json;
        ofLogWarning() << "Json config received: \n" << json.dump(4);
        loadFromJson(json);
        updateScenes();
        save();
    }
    catch (const std::exception &err) {
        ofLogError() << "Remote config load Failed: " << err.what();
    }
}
