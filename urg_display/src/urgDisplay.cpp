//
//  urgDisplay.cpp
//  urg_capture_display
//
//  Created by Ben Snell on 11/16/15.
//
//

#include "urgDisplay.h"

urgDisplay::urgDisplay() {
    
    // setup linear mesh
    linearMesh.setMode(OF_PRIMITIVE_POINTS);
    linearMesh.enableColors();
    
    // setup spherical mesh
    sphericalMesh.setMode(OF_PRIMITIVE_POINTS);
    sphericalMesh.enableColors();
    
    // setup gui
    linearParams.setName("Linear Mesh Parameters");
    linearParams.add(linearScale.set("Scale", 0.25, 0, 2));
    linearParams.add(linearSlide.set("Slide", 0, -10000, 100000));
    linearParams.add(linearSlideStep.set("Slide Step", 10, 1, 100));
    linearParams.add(linearAutoSlide.set("Auto Slide", false));
    linearParams.add(linearAutoSlideStep.set("Auto Step", 12, 0, 50));
    linearParams.add(linearXTranslation.set("X Translation", 0, -2000, 2000));
    linearParams.add(linearYTranslation.set("Y Translation", 0, -2000, 2000));
    linearParams.add(xRotation.set("X Rotation", 0, -180, 180));
    linearParams.add(yRotation.set("Y Rotation", 0, -180, 180));
    linearParams.add(zRotation.set("Z Rotation", 0, -180, 180));
    linearParams.add(mirrorX.set("Mirror X", false));
    linearParams.add(mirrorY.set("Mirror Y", false));
    linearParams.add(mirrorZ.set("Mirror Z", false));
    
    sphericalParams.setName("Spherical Mesh Params");
    sphericalParams.add(sphericalScale.set("Scale", 0.5, 0, 2));
    sphericalParams.add(sphericalRotation.set("Rotation", 0, -10000, 10000));
    sphericalParams.add(sphericalRotationStep.set("Rotation Step", 0.2, 0, 1));
    sphericalParams.add(sphericalAutoRotation.set("Auto Rotate", false));
    sphericalParams.add(sphericalAutoRotationStep.set("Auto Step", 0.05, -1, 1));
    sphericalParams.add(sphericalXTranslation.set("X Translation", 0, -2000, 2000));
    sphericalParams.add(sphericalYTranslation.set("Y Translation", 0, -2000, 2000));
    sphericalParams.add(flipX.set("Flip X", false));
    sphericalParams.add(flipY.set("Flip Y", false));
    sphericalParams.add(flipZ.set("Flip Z", false));
    
}

// ---------------------------------------------------------------------

void urgDisplay::loadLinearData(string fileName) {
    
    ofFile file(fileName);
    
    linearBuffer = ofBuffer(file);
}

// ---------------------------------------------------------------------

void urgDisplay::fillLinearMesh(int startScan, int endScan, int zScale, int minIndex, int maxIndex, bool timeDependent, int cullDistance, ofColor color) {

    // clear the existing mesh of any points
    linearMesh.clear();
    
    // reset number of scans
    nLinearScans = 0;
    
    // starting time of the first specified scan (seconds)
    float timeZero;
    
    // get iterator to the first scan
    ofBuffer::Line it = linearBuffer.getLines().begin();
    advance(it, startScan);
    // get iterator to the last scan
        // if endScan = -1, go to the end
        // if endScan is specified (not -1), then go up to that scan number
    ofBuffer::Line end = linearBuffer.getLines().end();
    if (endScan != -1) {
        end = linearBuffer.getLines().begin();
        advance(end, endScan - 1);
    }
    
    // iterate through all specified scans
    for (it; it != end; ++it) {
        
        // get the line and split it by commas
        string line = *it;
        // skip if line is empty
        if (line.empty() == true) continue;
        vector<string> items = ofSplitString(line, ",");
        
        // if time-dependent, find current time
        float timeNow;
        if (timeDependent) {
            if (nLinearScans == 0) {    // first scan
                timeZero = ofToFloat(items[0]) / 1000.;
                timeNow = 0;
            } else {                    // not first scan
                timeNow = ofToFloat(items[0]) / 1000. - timeZero;
            }
        }
        
        // add each specified point of the scan to the mesh
        // (add one because the first datum in the csv line is the time)
        for (int i = minIndex + 1; i < maxIndex + 1; i++) {
            
            // get the coordinates
            float px = ofToFloat(items[2 * i]); // millimeters
            float py = ofToFloat(items[2 * i + 1]);
            
            // if a cull distance is provided, calculate the distance of this point to the origin
            if (cullDistance != 0) {
                double distance = ofVec2f(px, py).distance(ofVec2f(0, 0));
                if (distance < abs(cullDistance)) continue;
            }
            
            // if time dependent, graph depth (pz) proportional to elapsed time; otherwise, graph with constant spacing (assume a new reading is taken every 100 ms)
            float pz = (timeDependent) ? (timeNow * zScale) : ((float)nLinearScans / 10. * zScale);
            
            // add the vertex to the mesh and add the specified color
            linearMesh.addVertex(ofVec3f(px, py, pz));
            linearMesh.addColor(ofFloatColor(1));
        }
        
        // increment scan number
        nLinearScans++;
    }
}

// ---------------------------------------------------------------------

void urgDisplay::drawLinearMesh() {

    // to use the mouseX and Y
    //    int mouseX = ofGetAppPtr()->mouseX;
    //    int mouseY = ofGetAppPtr()->mouseY;
    
    // update auto controls
    int key = getLKey();
    if (key == 's') linearAutoSlide = !linearAutoSlide;
    if (linearAutoSlide) linearSlide -= linearAutoSlideStep;
    
    // update manual controls
    if (key == OF_KEY_UP) linearScale *= 1.1;
    if (key == OF_KEY_DOWN) linearScale *= 0.9;
    if (key == OF_KEY_RIGHT) {
        if (linearAutoSlide) {
            linearAutoSlideStep -= 1;
        } else {
            linearSlide += linearSlideStep * 1/linearScale;
        }
    }
    if (key == OF_KEY_LEFT) {
        if (linearAutoSlide) {
            linearAutoSlideStep += 1;
        } else {
            linearSlide -= linearSlideStep * 1/linearScale;
        }
    }
    
    // update the lerp
    linearSlideLerp = linearSlideLerp * (1.-linearSlideLerpAmt) + linearSlide * linearSlideLerpAmt;
    
    // draw the mesh
    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2 + linearXTranslation, ofGetHeight() / 2 + linearYTranslation);
    
    ofRotateY(yRotation);
    ofRotateX(xRotation);
    ofRotateZ(zRotation);
    
    ofTranslate(0, 0, linearSlideLerp * linearScale);
    
    ofScale(1 - 2 * mirrorX, 1 - 2 * mirrorY, 1 - 2 * mirrorZ);
    ofScale(linearScale, linearScale, linearScale);
    
    linearMesh.drawVertices();
    ofPopMatrix();
    
}

// ---------------------------------------------------------------------

void urgDisplay::loadSphericalData(string fileName) {
    
    ofFile file(fileName);
    
    sphericalBuffer = ofBuffer(file);
}

// ---------------------------------------------------------------------

void urgDisplay::fillSphericalMesh(float speed, float period, float startingPeriod, float nPeriods, int minIndex, int maxIndex, bool clockwise, int cullDistance, float alignmentAngle, ofColor color, bool cullDuplicateScans) {
    
    // clear the existing mesh of any points
    sphericalMesh.clear();
    
    // reset number of scans
    nSphericalScans = 0;
    
    // if nPeriods = -1, then set it to an unreasonably high number
    if (nPeriods == -1) nPeriods = 99999.;
    
    // starting time of the first specified scan (seconds)
    float timeZero;

    // find the first scan within this period
    ofBuffer::Line it = sphericalBuffer.getLines().begin();
    ofBuffer::Line end = sphericalBuffer.getLines().end();
    while(it != end) {

        string line = *it;
        if (line.empty() == true) continue;
        vector<string> items = ofSplitString(line, ",");
        
        float timeNow = ofToFloat(items[0]) / 1000.;
        
        if (timeNow * speed >= startingPeriod * period) {
            timeZero = timeNow;
            // break so iterator begins at the starting period
            break;
        }
        
        // increment iterator
        ++it;
    }
    if (it == end) {
        cout << "Desired interval cannot be set. Try setting to a lower startingPeriod. Exiting..." << endl;
        ofExit();
    }
    
    // start adding points to the mesh and continue checking for an end condition
    float prevTime = -9999;
    while (it != end) {
        
        string line = *it;
        if (line.empty() == true) continue;
        vector<string> items = ofSplitString(line, ",");
        
        // find current time
        float timeNow = ofToFloat(items[0]) / 1000. - timeZero;
        
        // check if end condition is met (scan has traversed nPeriods)
        if (timeNow * speed > (startingPeriod + nPeriods) * period) break;
        
        // check if scan is a duplicate
        if (cullDuplicateScans) {
            float diff = timeNow - prevTime;
            if (diff <= 0.05) {
                ++it;
                continue;
            }
        }
        
        // add points to the mesh
        for (int i = minIndex + 1; i < maxIndex + 1; i++) {
            
            // get the coordinates
            float px = ofToFloat(items[2 * i]); // millimeters
            float py = ofToFloat(items[2 * i + 1]);
            
            // if a cull distance is provided, calculate the distance of this point to the origin
            if (cullDistance != 0) {
                double distance = ofVec2f(px, py).distance(ofVec2f(0, 0));
                if (distance < abs(cullDistance)) continue;
            }
            
            // rotate point about z axis to orient it upwards
            ofVec3f point(px, py, 0);
            point.rotate(180, ofVec3f(0, 0, 1));
            
            // apply the realignment angle to stretch or compress each chunk (period) of data
            float alignmentAmt = (float)i / 682. * alignmentAngle;
            point.rotate(alignmentAmt, ofVec3f(0, 0, 1));
            
            // rotate point about the y axis an amount proportional to the elapsed time and speed
            float rotationAmt = timeNow * speed;
            if (clockwise) rotationAmt *= -1.;
            point.rotate(rotationAmt, ofVec3f(0, 1, 0));
            
            // add point to the mesh
            sphericalMesh.addVertex(point);
            sphericalMesh.addColor(ofFloatColor(1));
        }
        
        prevTime = timeNow;
        nSphericalScans++;
        ++it;
    }
}

// ---------------------------------------------------------------------

void urgDisplay::drawSphericalMesh(bool cameraOn) {
    
    // to use the mouseX and Y
    //    int mouseX = ofGetAppPtr()->mouseX;
    //    int mouseY = ofGetAppPtr()->mouseY;
    
    // update auto controls
    int key = getSKey();
    if (key == 'r') sphericalAutoRotation = !sphericalAutoRotation;
    if (sphericalAutoRotation) sphericalRotation -= sphericalAutoRotationStep;
    
    // update manual controls
    if (key == OF_KEY_UP) sphericalScale *= 1.1;
    if (key == OF_KEY_DOWN) sphericalScale *= 0.9;
    if (key == OF_KEY_RIGHT) {
        if (sphericalAutoRotation) {
            sphericalAutoRotationStep -= 0.1;
        } else {
            sphericalRotation += sphericalRotationStep * 1/sphericalScale;
        }
    }
    if (key == OF_KEY_LEFT) {
        if (sphericalAutoRotation) {
            sphericalAutoRotationStep += 0.1;
        } else {
            sphericalRotation -= sphericalRotationStep * 1/sphericalScale;
        }
    }
    
    // update the lerp
    sphericalRotationLerp = sphericalRotationLerp * (1.-sphericalRotationLerpAmt) + sphericalRotation * sphericalRotationLerpAmt;
    
    // start the camera if bool is true (to be able to rotate mesh with cursor)
    if (cameraOn) easyCam.begin();
    
    // draw the mesh
    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2 + sphericalXTranslation, ofGetHeight() / 2 + sphericalYTranslation);
    
    ofScale(1 - 2 * flipX, 1 - 2 * flipY, 1 - 2 * flipZ);
    ofScale(sphericalScale, sphericalScale, sphericalScale);
    
    ofRotateY(sphericalRotationLerp);
    
    sphericalMesh.drawVertices();
    ofPopMatrix();
    
    if (cameraOn) easyCam.end();
}

// ---------------------------------------------------------------------

void urgDisplay::setKeyPressed(int key_) {
    lkey = skey = key_;
}

// ---------------------------------------------------------------------

int urgDisplay::getLKey() {
    int tempKey = lkey;
    lkey = 0;
    return tempKey;
}

// ---------------------------------------------------------------------

int urgDisplay::getSKey() {
    int tempKey = skey;
    skey = 0;
    return tempKey;
}

