#include "ofApp.h"

// by default, this app assumes linear data
// to use spherical data, uncomment this line:
//#define spherical

//--------------------------------------------------------------
void ofApp::setup(){

    // setup gui
    generalControls.setName("General Controls");
    generalControls.add(cursorShowing.set("Show Cursor", true));
    generalControls.add(debug.set("Debug", false));
    
    panel.setup();
    panel.add(generalControls);
    
    
#ifndef spherical
    
    // load data
    urg.loadLinearData("linear_test.csv");
    
    // fill mesh with the data
    urg.fillLinearMesh(0, -1, 300, 0, 682, false, 265, ofColor(255));
    
    // attach linear gui
    panel.add(urg.linearParams);
    
#else
    
    urg.loadSphericalData("spherical_test.csv");
    urg.fillSphericalMesh(225./64., 180, 0, 1, 0, 682, true, 265, 3, ofColor(255), true);
    panel.add(urg.sphericalParams);
    
#endif
    
    panel.loadFromFile("settings.xml");

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0);
    
#ifndef spherical
    
    // draw linear mesh
    urg.drawLinearMesh();
    
#else
    
    urg.drawSphericalMesh(false);
    
#endif
    
    if (debug) {
        panel.draw();
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10, 20);
    }
    ofDrawBitmapStringHighlight("L / R arrow keys slide linear or rotate spherical\nU / D arrow keys scale model up and down\nb for debug\ns for auto slide, r for auto rotate\nc to hide cursor\nf for fullscreen", 10, ofGetHeight() - 80);
}

//--------------------------------------------------------------
void ofApp::exit() {
    
    panel.saveToFile("settings.xml");
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    // pass the key to urg
    urg.setKeyPressed(key);
    
    if (key == 'f') ofToggleFullscreen();
    if (key == 'b') debug = !debug;
    if (key == 'c') {
        if (cursorShowing) {
            cursorShowing = false;
            ofHideCursor();
        } else {
            cursorShowing = true;
            ofShowCursor();
        }
    }
    
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
