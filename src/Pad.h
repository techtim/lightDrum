//
//  Pad.h
//  lightDrum
//
//  Created by Timofey Tavlintsev on 10/25/17.
//
//

#pragma once 
#include "ofMain.h"
#include "ofJson.h"

struct Pad {
    ofRectangle bounds;
    ofVec2f from;
    ofVec2f to;
    int pitch;
    float value;
    int velocity;
    uint64_t lastTrigTime;
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
