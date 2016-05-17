//
// Written by Ben Snell, 2016
//
// Recording app for the Hokuyo URG LIDAR
//
// Use with Dan Moore's ofxURG:
// https://github.com/danzeeeman/ofxUrg
//

#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxGui.h"
#include "urgRecorder.h"

class ofApp : public ofBaseApp {
public:
    
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    
    urgRecorder rec;
    
    ofxPanel panel;
    
    bool bDebug = true;
    
};
