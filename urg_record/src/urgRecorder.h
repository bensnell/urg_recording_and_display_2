//
//  urgRecorder.h
//  urg_record
//
//  Created by Ben Snell on 5/16/16.
//
//  Coupled with Dan Moore's ofxURG, records LIDAR data points to CSV
//  and displays a real-time rendering of the data being recorded
//  for debugging and for fun!
//
//  ofxURG: https://github.com/danzeeeman/ofxUrg
//

#ifndef __urg_record__urgRecorder__
#define __urg_record__urgRecorder__

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOsc.h"

class urgRecorder {
    
public:
    
    urgRecorder();
    
    // --------------------- GUI ---------------------
    
    ofParameter<bool> drawRender;
    ofParameter<bool> spherical;
    ofParameter<float> renderScale;
    ofParameter<int> zSpacing; // only for rendering linear
    ofParameter<int> nMeshes;
    ofParameter<int> xTranslate;
    ofParameter<int> yTranslate;
    ofParameter<int> zRotation; // x rotation in spherical
    ofParameter<bool> mirror; // for linear, flips XY plane; for spherical, switches direction
    ofParameterGroup renderParams;
    
    ofParameter<bool> startRecording;   // flag to start recording
    ofParameter<bool> stopRecording;    // flag to stop recording
    ofParameter<bool> recordingState;
    ofParameter<bool> liveData;         // whether we're currently getting data
    ofParameterGroup recordingParams;
    
    // ------------ CONNECT OSC -------------
    
    // connect to specified port
    // default port is the same port specified in settings.xml of ofxURG's sender app
    void setup(int port = 7777);
    int oscPort;
    
    // receives data over osc
    ofxOscReceiver receiver;
    
    // determine if we're getting data
    unsigned long lastDataTime = 0;
    int dataTimeout = 1000; // ms
    
    
    
    // ------------ RECORD DATA -------------
    
    void update();
    
    // file where data is recorded to
    ofFile recFile;
    /* format of data (time in milliseconds, points in millimeters):
        time    x1     y1      x2      y2      x3      y3  ...
        .
        .
        .
     */
    
    // counter of number of scans recorded to file
    unsigned long scanCounter = 0;
    
    // mesh to store the most recent scan (in XY plane)
    ofMesh lastScan;
    /* Orientation of data received:
           y
           |
         . | .
        \  |    '
         \ |      '
          \|       .
           o ---------- x
          /        ,
         /        .
        /       ,
         '   '
    */
    
    // stores last nMeshes meshes for a realtime render of recent scans
    vector<ofMesh> scans;
    
    // stores the beginning time of a recording
    unsigned long timeZero;
    
    // last 341 reading
    float lastSample = 0.;
    
    // tracker data points
    map<int, ofVec2f> points;
    
    // ------------ RENDER DATA -------------
    
    // draw the point data to screen as it's being received or recorded
    void draw();
    
    // FOR SPHERICAL SCANS:
    // current assumed rotation of lidar
    float rotation = 0.;
    // rotation step of lidar (horizontal resolution)
    float rotationStep = 360. / 1024.;
    // number of motor steps per one relative 0.35 degree reference step (min  = 1; max = 8) (default = 1) --> must change in Arduino code too
    int stepResolution = 1;
    // flip the direction of the scan
    int flipDirection = 1; // false if 1
    
};


#endif /* defined(__urg_record__urgRecorder__) */
