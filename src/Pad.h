//
//  Pad.h
//  lightDrum
//
//  Created by Timofey Tavlintsev on 10/25/17.
//
//

#pragma once
#include "ofJson.h"
#include "ofMain.h"

struct Pad {
    Pad()
        : bounds(0, 0, 0, 0)
        , from(0, 0)
        , to(0, 0)
        , pitch(0)
        , value(0)
        , velocity(0)
        , lastTrigTime(0)
    {
    }

    Pad(ofRectangle _bounds, ofVec2f _from, ofVec2f _to, int _pitch = 0, float _value = 0,
        float _velocity = 0, uint64_t _lastTrigTime = 0)
        : bounds(_bounds)
        , from(_from)
        , to(_to)
        , pitch(_pitch)
        , value(_value)
        , velocity(_velocity)
        , lastTrigTime(_lastTrigTime)
    {
    }

    ofRectangle bounds;
    ofVec2f from;
    ofVec2f to;
    int pitch;
    float value, velocity;
    uint64_t lastTrigTime;
};

static void to_json(ofJson &j, const Pad &p)
{
    j = ofJson{ { "from", { { "x", p.from.x }, { "y", p.from.y } } },
                { "to", { { "x", p.to.x }, { "y", p.to.y } } },
                { "bounds",
                  { { "x", p.bounds.x },
                    { "y", p.bounds.y },
                    { "w", p.bounds.getWidth() },
                    { "h", p.bounds.getHeight() } } },
                { "pitch", p.pitch } };
}

static void from_json(const ofJson &j, Pad &p)
{
    if (j.count("from") > 0) {
        p.from.x = j.at("from").at("x").get<float>();
        p.from.y = j.at("from").at("y").get<float>();
    }
    if (j.count("to") > 0) {
        p.to.x = j.at("to").at("x").get<float>();
        p.to.y = j.at("to").at("y").get<float>();
    }
    if (j.count("pitch") > 0) {
        p.pitch = j.at("pitch").get<int>();
    }
    if (j.count("bounds") > 0)
        p.bounds = ofRectangle(j.at("bounds").at("x").get<float>(),
            j.at("bounds").at("y").get<float>(),
            j.at("bounds").at("w").get<float>(),
            j.at("bounds").at("h").get<float>());

    p.lastTrigTime = 0;
}
