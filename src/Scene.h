//
//  Scene.h
//  lightDrum
//
//  Created by Timofey Tavlintsev on 10/23/17.
//
//
#pragma once
#include "Pad.h"
#include "ofMain.h"
#include "ofxDatGui.h"
#include <regex>

enum VALUE_ROUTE : uint8_t {
    VALUE_TO_NONE = 0,
    VALUE_TO_COLOR = 1,
    VALUE_TO_POSITION = 2,
    VALUE_TO_BRIGHT = 3
};
const static vector<string> s_PadValueToStr
    = { "to none", "to color", "to position", "to brightness" };

static const float ONE_TO_127 = 1.0f / 127.0f;
static const string LDGUI_VALUE_ROUTE_ENV = "ADSR route to";
static const string LDGUI_VALUE_ROUTE_VEL = "Velocity route to";
const static string s_ShaderFolder = "shaders/";
static vector<string> s_shadersList;

class Scene {
    //    ofShader m_shader;
    bool m_bShaderLoad = false;

public:
    size_t m_id;
    bool m_enable;
    ofVec4f m_adsr;
    ofColor m_color1, m_color2;
    string m_shaderName;
    uint8_t m_routeEnvelope;
    uint8_t m_routeVelocity;

    Scene(size_t _id = 0)
    : m_bShaderLoad(false)
    , m_id(_id)
    , m_enable(false)
    , m_adsr(200, 0, 0, 200)
    , m_color1(255)
    , m_color2(255)
    , m_shaderName("default")
    , m_routeEnvelope(1)
    , m_routeVelocity(0)
    {
    }
    
    static unique_ptr<ofxDatGui> GenerateGui()
    {
        ofLogVerbose() << "Generate scene gui";

        unique_ptr<ofxDatGui> gui = make_unique<ofxDatGui>();
        unique_ptr<ofxDatGuiThemeCharcoal> guiTheme = make_unique<ofxDatGuiThemeCharcoal>();
        gui->setTheme(guiTheme.get(), true);
        gui->addHeader("Scene");
        gui->setWidth(200);
        gui->setPosition(ofGetWidth() - 200, ofGetHeight() / 2 - 20);
        gui->setAutoDraw(false);

        gui->addColorPicker("color 1");
        gui->addColorPicker("color 2");

        gui->addLabel("ADSR Envelope in ms");
        auto slider = gui->addSlider("attack", 0, 1000.0);
        slider->setPrecision(0);
        slider = gui->addSlider("decay", 0, 1000.0);
        slider->setPrecision(0);
        slider = gui->addSlider("sustain", 0, 1000.0);
        slider->setPrecision(0);
        slider = gui->addSlider("release", 0, 1000.0);
        slider->setPrecision(0);

        gui->addDropdown(LDGUI_VALUE_ROUTE_ENV, s_PadValueToStr);

        gui->addLabel("MIDI Velocity");
        gui->addDropdown(LDGUI_VALUE_ROUTE_VEL, s_PadValueToStr);

        /// Shaders
        if (s_shadersList.empty()) {
        }

        gui->setEnabled(true);
        gui->setVisible(true);

        return move(gui);
    }

    void bindGui(ofxDatGui *gui)
    {
        gui->getSlider("attack")->bind(m_adsr.x);
        gui->getSlider("decay")->bind(m_adsr.y);
        gui->getSlider("sustain")->bind(m_adsr.z);
        gui->getSlider("release")->bind(m_adsr.w);
        /// bind first update widget color after
        gui->getColorPicker("color 1")->bind(m_color1);
        gui->getColorPicker("color 1")->setColor(m_color1);
        gui->getColorPicker("color 2")->bind(m_color2);
        gui->getColorPicker("color 2")->setColor(m_color2);

        auto dropDown = gui->getDropdown(LDGUI_VALUE_ROUTE_ENV);
        if (dropDown->getName() != "X") {
            dropDown->onDropdownEvent(this, &Scene::onDropdownEvent);
            dropDown->select(m_routeEnvelope);
        }

        dropDown = gui->getDropdown(LDGUI_VALUE_ROUTE_VEL);
        if (dropDown->getName() != "X") {
            dropDown->onDropdownEvent(this, &Scene::onDropdownEvent);
            dropDown->select(m_routeVelocity);
        }
    }

    void updateShader()
    {
        if (m_bShaderLoad) {
            return;
        }

        auto path = s_ShaderFolder + m_shaderName;
        //        m_bShaderLoad = m_shader.load(path + ".vs", path + ".fs");
    }

    bool setupShader(const string &name)
    {
        if (m_shaderName == name) {
            return false;
        }
        m_shaderName = name;
        m_bShaderLoad = false;
    }

    void reloadShader() { m_bShaderLoad = false; }

    void updateAndDraw(Pad &pad) const
    {
        auto durationMs = ofGetSystemTime() - pad.lastTrigTime;
        ofNoFill();
        ofSetColor(100);
        ofDrawRectangle(pad.bounds.x - 1, pad.bounds.y - 1, pad.bounds.width + 2,
                        pad.bounds.height + 2);
        ofFill();

        ofColor color(0, 0, 0, 255);

        pad.value = getADSRValue(m_adsr, durationMs);
        if (pad.value == 0) {
            /// draw black rect and return when envelope is inactive
            ofSetColor(color);
            ofDrawRectangle(pad.bounds);
            return;
        }

        if (m_routeEnvelope == VALUE_TO_POSITION) {
            auto pos = pad.from.getInterpolated(pad.to, pad.value);
            color = m_color1.getLerped(m_color2, pad.value);
            ofSetColor(color);
            ofDrawRectangle(pos.x - 10, pos.y - 15, 20, 15);
            return;
        }
        else if (m_routeEnvelope == VALUE_TO_COLOR) {
            color = m_color1.getLerped(m_color2, pad.value);
        }

        if (m_routeVelocity == VALUE_TO_POSITION) {
            auto pos = pad.from.getInterpolated(pad.to, pad.velocity);
            color = m_color1.getLerped(m_color2, pad.value);
            ofSetColor(color);
            ofDrawRectangle(pos.x - 10, pos.y - 25, 20, 25);
            return;
        }
        else if (m_routeVelocity == VALUE_TO_COLOR) {
            color = m_color1.getLerped(m_color2, pad.velocity);
        }

        if (m_routeEnvelope == VALUE_TO_BRIGHT) {
            //            color = ofColor(color, pad.value * 255.0f);
            color.setBrightness(pad.value * 255.0f);
        }
        if (m_routeVelocity == VALUE_TO_BRIGHT) {
            //            color = ofColor(color, pad.velocity * 255.0f);
            color.setBrightness(pad.velocity * 255.0f);
        }

        /// shaders in progress
        /*
        if (false) {
            updateShader();
            if (m_bShaderLoad) {
                m_shader.begin();
                // we want to pass in some varrying values to animate our type / color
                m_shader.setUniform4f("color1", m_color1);
                m_shader.setUniform4f("color2", m_color2);
                m_shader.setUniform1f("velocity", pad.velocity);
                m_shader.setUniform1f("envelope", pad.value);
                // m_shader.setUniform2f("texSize", ofVec2f(pad.bounds.width, pad.bounds.height));
                m_shader.setUniform1i("envelopeTo", static_cast<int>(m_routeEnvelope));
                m_shader.setUniform1i("velocityTo", static_cast<int>(m_routeVelocity));
                ofDrawRectangle(pad.bounds);
                m_shader.end();
            }
        }
         */
        ofSetColor(color);
        ofDrawRectangle(pad.bounds);
    }

    float getADSRValue(const ofVec4f &adsr, const uint64_t &durationMs) const
    {
        auto totalAdsr = adsr.x + adsr.y + adsr.z + adsr.w;
        float ADSRvalue = 0;

        if (totalAdsr < durationMs || adsr == ofVec4f(0.0)) {
            return ADSRvalue;
        }
        else if (durationMs <= adsr.x) { // R attack
            ADSRvalue = durationMs / adsr.x;
            if (ADSRvalue > 1.0)
                ADSRvalue = 1.0;
        }
        else if (durationMs > adsr.x && durationMs <= adsr.x + adsr.z) { // Z sustain
            ADSRvalue = 1.0;
        }
        else if (adsr.w != 0 && durationMs <= adsr.x + adsr.z + adsr.w) { // W release
            ADSRvalue = 1.0 - (durationMs - adsr.x - adsr.z) / adsr.w;
            if (ADSRvalue < 0)
                ADSRvalue = 0;
        }

        return ADSRvalue;
    }

    void onDropdownEvent(ofxDatGuiDropdownEvent e)
    {
        if (e.target->getName() == LDGUI_VALUE_ROUTE_ENV) {
            m_routeEnvelope = static_cast<uint8_t>(e.child);
            ofLogVerbose("Drop Down ENV" + ofToString(e.child));
        }
        if (e.target->getName() == LDGUI_VALUE_ROUTE_VEL) {
            m_routeVelocity = static_cast<uint8_t>(e.child);
            ofLogVerbose("Drop Down VEL" + ofToString(e.child));
        }
    }

    void loadShaders()
    {
        s_shadersList.clear();

        ofDirectory dir(s_ShaderFolder);
        if (!dir.exists()) {
            dir.createDirectory(s_ShaderFolder);
        }
        dir.listDir();
        dir.sort();
        regex ctrl_name("(.+)\\.vs"); // ([^\\.]+)
        smatch base_match;
        for (size_t i = 0; i < dir.size(); i++) {
            string fileName = dir.getFile(i).getBaseName();
            regex_match(fileName, base_match, ctrl_name);
            /// search for *.vs files
            if (base_match.size() < 2)
                continue;
            string fragName = base_match[1].str() + ".fs";
            /// check for *.fs file with same name
            if (find_if(dir.getFiles().begin(), dir.getFiles().end(),
                        [&fragName](const ofFile &file) { return file.getBaseName() == fragName; })
                != dir.getFiles().end()) {
                ofLogVerbose() << "Scene: add shader =" << base_match[1].str();
                s_shadersList.push_back(base_match[1].str());
            }
        }
    }
};

static void to_json(ofJson &j, const Scene &scene)
{
    j = ofJson{
        { "adsr",
          { { "a", scene.m_adsr.x },
            { "d", scene.m_adsr.y },
            { "s", scene.m_adsr.z },
            { "r", scene.m_adsr.w } } },
        { "color1",
          { { "r", scene.m_color1.r }, { "g", scene.m_color1.g }, { "b", scene.m_color1.b } } },
        { "color2",
          { { "r", scene.m_color2.r }, { "g", scene.m_color2.g }, { "b", scene.m_color2.b } } },
        { "shaderName", scene.m_shaderName },
        { "routeVelocity", scene.m_routeVelocity },
        { "routeEnvelope", scene.m_routeEnvelope }
    };
}

static void from_json(const ofJson &j, Scene &scene)
{
    ofLogVerbose() << "Load Scene";

    if (j.count("adsr") > 0) {
        scene.m_adsr.x = j.at("adsr").at("a").get<float>();
        scene.m_adsr.y = j.at("adsr").at("d").get<float>();
        scene.m_adsr.z = j.at("adsr").at("s").get<float>();
        scene.m_adsr.w = j.at("adsr").at("r").get<float>();
    }
    if (j.count("color1") > 0) {
        scene.m_color1.set(j.at("color1").at("r").get<float>(), j.at("color1").at("g").get<float>(),
                           j.at("color1").at("b").get<float>(), 255);
        ofLogVerbose() << "color1 = " << scene.m_color1;
    }
    if (j.count("color2") > 0) {
        scene.m_color2.set(j.at("color2").at("r").get<float>(), j.at("color2").at("g").get<float>(),
                           j.at("color2").at("b").get<float>(), 255);
        ofLogVerbose() << "color2 = " << scene.m_color2;
    }

    if (j.count("shaderName") > 0) {
        scene.m_shaderName = j.at("shaderName").get<string>();
        ofLogVerbose() << "shaderName = " << scene.m_shaderName;
    }

    if (j.count("routeVelocity")) {
        scene.m_routeVelocity = j.at("routeVelocity").get<uint8_t>();
        ofLogVerbose() << "routeVelocity = " << scene.m_routeVelocity;
    }

    if (j.count("routeEnvelope")) {
        scene.m_routeEnvelope = j.at("routeEnvelope").get<uint8_t>();
        ofLogVerbose() << "routeEnvelope = " << scene.m_routeEnvelope;
    }
}
