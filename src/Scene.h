//
//  Scene.h
//  lightDrum
//
//  Created by Timofey Tavlintsev on 10/23/17.
//
//
#pragma once
#include "ofMain.h"
#include "ofxDatGui.h"

#include "Pad.h"

enum PAD_VALUE_ROUTE { PAD_VALUE_TO_COLOR, PAD_VALUE_TO_POSITION, PAD_VALUE_TO_BRIGHT };
const static map<int, string> PadValueToStr = { { PAD_VALUE_TO_COLOR, "to color" },
                                                { PAD_VALUE_TO_POSITION, "to position" },
                                                { PAD_VALUE_TO_BRIGHT, "to brightness" } };

class Scene {
    unique_ptr<ofxDatGui> m_gui;
    unique_ptr<ofxDatGuiTheme> m_guiTheme;
    ofShader m_shader;
    
public:
    Scene(size_t _id = 0)
        : m_id(_id)
        , m_enable(false)
        , m_adsr(200,0,0,200)
        , m_color1(255)
        , m_color2(255)
        , m_shaderPath("default")
    {
        setupGui();
    }

    Scene(const Scene &other)
        : m_id(other.m_id)
        , m_enable(false)
        , m_adsr(other.m_adsr)
        , m_color1(other.m_color1)
        , m_color2(other.m_color2)
        , m_shaderPath(other.m_shaderPath)
    {
        setupGui();
    }

    Scene(Scene &&other)
        : m_adsr(move(other.m_adsr))
        , m_color1(move(other.m_color1))
        , m_color2(move(other.m_color2))
        , m_shaderPath(move(other.m_shaderPath))
    {
        setupGui();
    }

    bool updateShader(const string &name = "default") {
        auto path = "shaders/" + name;
    }
    
    void setEnable(bool bEnable)
    {
        if (m_enable == bEnable)
            return;

        m_enable = bEnable;
        m_gui->setEnabled(m_enable);
        m_gui->setVisible(m_enable);
    }

    void setupGui()
    {
        m_gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::BOTTOM_RIGHT);
        m_guiTheme = make_unique<ofxDatGuiThemeCharcoal>();
        m_gui->setTheme(m_guiTheme.get());
        m_gui->addHeader("Scene");
        m_gui->setWidth(200);
        m_gui->setPosition(ofGetWidth() - 200, ofGetHeight() / 2);

        auto picker = m_gui->addColorPicker("color 1");
        picker->bind(m_color1);
        picker = m_gui->addColorPicker("color 2");
        picker->bind(m_color2);

        m_gui->addLabel("ADSR Envelope in ms");
        auto slider = m_gui->addSlider("attack", 0, 1000.0);
        slider->bind(m_adsr.x);
        slider->setPrecision(0);
        slider = m_gui->addSlider("decay", 0, 1000.0);
        slider->bind(m_adsr.y);
        slider->setPrecision(0);
        slider = m_gui->addSlider("sustain", 0, 1000.0);
        slider->bind(m_adsr.z);
        slider->setPrecision(0);
        slider = m_gui->addSlider("release", 0, 1000.0);
        slider->bind(m_adsr.w);
        slider->setPrecision(0);
    }

    void update() { m_gui->update(); }

    void draw() { m_gui->draw(); }

    void updateAndDraw(Pad &pad)
    {
        auto durationMs = ofGetSystemTime() - pad.lastTrigTime;
        ofNoFill();
        ofSetColor(100);
        ofDrawRectangle(pad.bounds.x-1, pad.bounds.y-1, pad.bounds.width+2, pad.bounds.height+2);
        ofFill();
        pad.value = getADSRValue(m_adsr, durationMs);
        if (pad.value == 0) {
            ofSetColor(0);
            ofDrawRectangle(pad.bounds);
            return;
        }
        auto pos = pad.from.getInterpolated(pad.to, pad.value);
        ofSetColor(m_color1.getLerped(m_color2, pad.value));
        ofDrawRectangle(pos.x - 10, pos.y - 20, 20, 20);
    }

    float getADSRValue(const ofVec4f &adsr, const uint64_t &durationMs)
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
        else if (durationMs > adsr.x && durationMs <= adsr.x + adsr.y) { // G decay
            ADSRvalue = 1.0;
        }
        else if (adsr.w != 0 && durationMs <= adsr.x + adsr.y + adsr.w) { // W release
            ADSRvalue = 1.0 - (durationMs - adsr.x - adsr.y) / adsr.w;
            if (ADSRvalue < 0)
                ADSRvalue = 0;
        }

        return ADSRvalue;
    }

    size_t m_id;
    bool m_enable;
    ofVec4f m_adsr;
    ofColor m_color1, m_color2;
    string m_shaderPath;
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
        { "shaderPath", scene.m_shaderPath }
    };
}

static void from_json(const ofJson &j, Scene &scene)
{
    scene.m_adsr.x = j.at("adsr").at("a").get<float>();
    scene.m_adsr.y = j.at("adsr").at("d").get<float>();
    scene.m_adsr.z = j.at("adsr").at("s").get<float>();
    scene.m_adsr.w = j.at("adsr").at("r").get<float>();

    scene.m_color1.r = j.at("color1").at("r").get<float>();
    scene.m_color1.g = j.at("color1").at("g").get<float>();
    scene.m_color1.b = j.at("color1").at("b").get<float>();

    scene.m_color2.r = j.at("color2").at("r").get<float>();
    scene.m_color2.g = j.at("color2").at("g").get<float>();
    scene.m_color2.b = j.at("color2").at("b").get<float>();

    scene.m_shaderPath = j.at("shaderPath").get<string>();
}
