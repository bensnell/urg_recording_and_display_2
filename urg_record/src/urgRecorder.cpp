//
//  urgRecorder.cpp
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

#include "urgRecorder.h"

urgRecorder::urgRecorder() {
    
    // setup gui
    
    renderParams.setName("Real-Time Render");
    renderParams.add(drawRender.set("Draw Real-Time Render", false));
    renderParams.add(spherical.set("Spherical", false));
    renderParams.add(zSpacing.set("Z Spacing", 100, 1, 400));
    renderParams.add(nMeshes.set("Number of Layers", 20, 1, 512));
    renderParams.add(renderScale.set("Rendering Scale", 0.2, 0.01, 1.));
    renderParams.add(xTranslate.set("X Translation", 0, -1000, 1000));
    renderParams.add(yTranslate.set("Y Translation", 0, -1000, 1000));
    renderParams.add(zRotation.set("Z Rotation", 0, -180, 180));
    renderParams.add(mirror.set("Mirror / Reverse", false));
    
    recordingParams.add(startRecording.set("Start New Recording", false));
    recordingParams.add(stopRecording.set("Stop Recording", false));
    recordingParams.add(recordingState.set("Recording State", false));
    recordingParams.add(liveData.set("Live Data", false));
    
}

//--------------------------------------------------------------

void urgRecorder::setup(int port) {
    
    oscPort = port;
    
    // connect to osc
    receiver.setup(oscPort);
    
}

//--------------------------------------------------------------

void urgRecorder::update() {
    
    // check to start new recording
    
    if (startRecording) {
        startRecording = false;
        
        // create a timestamped title and a new file
        string fileName = ofGetTimestampString() + "_recording" + ".csv";
        recFile.open(ofToDataPath(fileName), ofFile::WriteOnly);
        
        // set recordingState to true
        recordingState = true;
        
        // reset the counter of the number of scans received
        scanCounter = 0;
        
        // clear meshes for realtime render
        scans.clear();
    }
    
    
    // check to stop current recording
    
    if (stopRecording) {
        stopRecording = false;
        
        // close file
        recFile.close();
        
        recordingState = false;
    }
    
    // set flip direction for spherical capture
    flipDirection = (mirror) ? -1 : 1;
    
    // get the data if there are waiting messages
    while(receiver.hasWaitingMessages()){
        
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        // check for point dataa
        if(m.getAddress() == "/urg/raw/data"){
            
            // mark that we're getting data
            lastDataTime = ofGetElapsedTimeMillis();
            
            // reset the last scan
            lastScan.clear();
            
            // find the current time
            unsigned long thisTime;
            if (scanCounter == 0) {
                // if this is the first scan we're recording, set timeZero
                timeZero = ofGetElapsedTimeMillis();
                thisTime = 0;
            } else {
                thisTime = ofGetElapsedTimeMillis() - timeZero;
            }
            
            // ---------- GET AND STORE THE DATA ---------
            
            // if we're recording data, write time to file
            if (recordingState) {
                recFile << ofToString(thisTime) << ",";
            }
            
            long sum = 0;
            
            // write arguments of message to file
            for(int i = 0; i < m.getNumArgs(); i+=2){
                float r = m.getArgAsInt32(i);
                float theta = m.getArgAsFloat(i+1);
                
                float x = r * cos(theta); // convert to cartesian coordinates
                float y = r * sin(theta);
                
                // if we're recording data, write points to file
                if (recordingState) {
                    if (i != 0) recFile << ",";
                    recFile << ofToString(x) + "," + ofToString(y);
                }
                
                // check if we're receiving data by summing all of it
                sum += r;
                
                // add point to mesh in XY plane
                lastScan.addVertex(ofVec3f(x, y, 0.));
            }
            
            liveData = (sum == 0) ? false : true;
            
            // add this mesh to the last scans
            scans.insert(scans.begin(), lastScan);
            while(scans.size() > nMeshes) { // resize to nMeshes
                scans.pop_back();
            }
            
            // increment scan counter
            scanCounter++;
            
            // if we're recording data, add a line break
            if (recordingState) {
                recFile << "\n";
            }
        }
        
        // check for tracker data
//        if(m.getAddress() == "/urg/tracker/data"){
//            for(int i = 0; i < m.getNumArgs(); i+=3){
//                points[m.getArgAsInt32(i)] = ofVec2f(m.getArgAsInt32(i+1), m.getArgAsInt32(i+2));
//            }
//        }
        
        // if we're rendering, increment thisAngle
        if (drawRender) {
//            cout << "here:\t" << (float)flipDirection * rotationStep / (float)stepResolution << endl;
            rotation += (float)flipDirection * rotationStep / (float)stepResolution;
            if (rotation > 360.) rotation = fmod(rotation, 360.f);
        }
    }
    
    if (ofGetElapsedTimeMillis() - lastDataTime > dataTimeout) liveData = false;
    
}

//--------------------------------------------------------------

void urgRecorder::draw() {
    
    // draw real-time render
    if (drawRender) {
        
        ofPushMatrix();
        ofTranslate(ofGetWidth() / 2. + xTranslate, ofGetHeight() / 2. + yTranslate, 0.);
        ofScale(renderScale, renderScale, renderScale);
        
        // if not drawing spherical 360 render, render linear spans by translating each scan backwards in z space
        if (!spherical) {
            
            // draw each mesh to screen, increasingly further back
            for (int i = 0; i < scans.size(); i++) {
                
                ofPushMatrix();
                ofTranslate(0., 0., - i * zSpacing / 2);
                ofRotateZ(zRotation);
                if (mirror) ofRotateY(180);
                scans[i].drawVertices();
                ofPopMatrix();
            }
        }
        
        // otherwise, draw data as spherical
        else {
            
            for (int i = 0; i < scans.size(); i++) {
                
                ofPushMatrix();
                ofRotateZ(-90.);
                ofRotateX(zRotation);
                
                // rotate to thisAngle + the step of this scan
                ofRotateX(rotation - (float)flipDirection * 2 * i * rotationStep / (float)stepResolution);
                
                scans[i].drawVertices();
                ofPopMatrix();
            }
        }
        
        ofPopMatrix();
    }
    
}
