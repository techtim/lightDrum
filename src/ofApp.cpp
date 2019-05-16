#include "ofApp.h"

static const string LD_ENABLE_LED_MAP = "enable led mapping";

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetVerticalSync(true);
    ofBackground(0, 0, 0);
    ofSetFrameRate(60);

#ifndef NDEBUG
    ofSetLogLevel(OF_LOG_VERBOSE);
#else
    ofSetLogLevel(OF_LOG_WARNING);
#endif

/// Set data path to Resources
#ifdef WIN32
    // no-op
#elif defined(__APPLE__)
    ofSetDataPathRoot("../Resources/");
#elif defined(TARGET_LINUX)
    // no-op
#endif


    ofSetEscapeQuitsApp(false);
    
    // print input ports to console
    m_midiIn.listInPorts(); // via instance
    //setMidiPort(m_midiIn.getNumPorts() - 1);
    setMidiPort(m_drum.getMidiDevice());
    // midiIn.openPort("IAC Pure Data In"); // by name

    // don't ignore sysex, timing, & active sense messages,
    // these are ignored by default
    m_midiIn.ignoreTypes(true, false, false);
    m_midiIn.addListener(this);
    m_midiIn.setVerbose(true);

    ofSetLogLevel(OF_LOG_WARNING);
}

//--------------------------------------------------------------
void ofApp::update() {
    m_drum.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    m_drum.draw();

    ofSetColor(255, 255, 255, 200);
    // draw the last recieved message contents to the screen
    text << "From MIDI device:" << m_midiPortName << " Received: " << ofxMidiMessage::getStatusString(m_midiMessage.status);
    ofDrawBitmapString(text.str(), 20, 20);
    text.str(""); // clear

    ofSetColor(100, 255, 255, 255);

    text << "channel: " << m_midiMessage.channel << " pitch: " << m_midiMessage.pitch
    << " velocity: " << m_midiMessage.velocity;
    ofDrawBitmapString(text.str(), 20,
                       40);
    text.str(""); // clear

#if !defined(TARGET_RASPBERRY_PI)
    ofSetWindowTitle("lightDrum (fps: " + ofToString(static_cast<int>(ofGetFrameRate())) + ")");
#endif
}

//--------------------------------------------------------------
void ofApp::exit()
{
    // clean up
    m_midiIn.closePort();
    m_midiIn.removeListener(this);
}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage &msg)
{
    // make a copy of the latest message
    m_midiMessage = msg;
    m_drum.onMidiMessage(msg);
}

bool ofApp::setMidiPort(size_t port)
{
    if (port < m_midiIn.getInPortList().size()) {
        m_midiPortNum = port;
        m_midiPortName = m_midiIn.getInPortList().at(port);
        m_midiIn.openPort(port);
        return true;
    }
    return false;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{

    switch (key) {
        case 'i':
            m_midiIn.listInPorts();
            break;
        case 'r':
            m_drum.reloadShader();
            break;
        case OF_KEY_UP:
            ++m_midiPortNum;
            if (!setMidiPort(m_midiPortNum))
                --m_midiPortNum;
            break;
        case OF_KEY_DOWN:
            --m_midiPortNum;
            ;
            if (!setMidiPort(m_midiPortNum))
                ++m_midiPortNum;
            break;
        case 's':
            m_drum.save(m_midiPortNum);
            break;
        case 'l':
            m_drum.load();
            setMidiPort(m_drum.getMidiDevice());
            break;
        case '1':
            m_midiMessage = makeFakeMidi(12, m_drum.getMidiChannel());
            m_drum.onMidiMessage(m_midiMessage);
            break;
        case '2':
           m_midiMessage = makeFakeMidi(13, m_drum.getMidiChannel());
           m_drum.onMidiMessage(m_midiMessage);
            break;
        case '3':
           m_midiMessage = makeFakeMidi(14, m_drum.getMidiChannel());
           m_drum.onMidiMessage(m_midiMessage);
            break;
        case '4':
           m_midiMessage = makeFakeMidi(19, m_drum.getMidiChannel());
           m_drum.onMidiMessage(m_midiMessage);
            break;
        case '5':
           m_midiMessage = makeFakeMidi(20, m_drum.getMidiChannel());
           m_drum.onMidiMessage(m_midiMessage);
            break;
        case '6':
           m_midiMessage = makeFakeMidi(21, m_drum.getMidiChannel());
           m_drum.onMidiMessage(m_midiMessage);
            break;
        default:
            break;
    }
}

ofxMidiMessage ofApp::makeFakeMidi(int pitch, int channel) {
    ofxMidiMessage message;
    message.pitch = pitch;
    message.channel = channel;
    message.velocity = ofRandom(10, 125);
    message.status = MIDI_NOTE_ON;
    return message;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {}

