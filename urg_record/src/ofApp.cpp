#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    
    // setup gui
    panel.setup();
    panel.add(rec.renderParams);
    panel.add(rec.recordingParams);
    
    // load last settings used
    panel.loadFromFile("settings.xml");
    
    // start listening over osc to port 7777
    rec.setup(7777);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    // update the recorder to record data if we're recording to file
    rec.update();
    
}


//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0);
    
    // draw the point data
    rec.draw();
    
    // debug app
    if (bDebug) {
        panel.draw(); // draw gui
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10, 20); // draw framerate
    }
}

//--------------------------------------------------------------
void ofApp::exit() {
    
    // when the app quits (Ctrl + Q), save these settings to be used next time
    panel.saveToFile("settings.xml");
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    // press f to view app fullscreen
    if (key == 'f') ofToggleFullscreen();
    
    // press b to hide and show the debug parameters
    if (key == 'b') bDebug = !bDebug;
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
