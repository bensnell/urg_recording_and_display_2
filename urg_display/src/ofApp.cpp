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
    urg.loadLinearData("2015-11-17 13-47-51 recording.csv");
    
    // fill mesh with the data
    urg.fillLinearMesh();
    
    // attach linear gui
    panel.add(urg.linearParams);
    
#else
    
    urg.loadSphericalData(<#string fileName#>);
    urg.fillSphericalMesh();
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
    
    urg.drawSphericalMesh();
    
#endif
    
    if (debug) {
        panel.draw();
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10, 20);
    }
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
