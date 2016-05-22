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
    linearParams.add(linearScale.set("Scale", 1, 0, 2));
    linearParams.add(linearSlide.set("Slide", 0, -10000, 100000));
    linearParams.add(linearSlideStep.set("Slide Step", 10, 1, 100));
    linearParams.add(linearAutoSlide.set("Auto Slide", false));
    linearParams.add(linearAutoSlideStep.set("Auto Step", 12, 0, 50));
    linearParams.add(xTranslation.set("X Translation", 0, -2000, 2000));
    linearParams.add(yTranslation.set("Y Translation", 0, -2000, 2000));
    linearParams.add(xRotation.set("X Rotation", 0, -180, 180));
    linearParams.add(yRotation.set("Y Rotation", 0, -180, 180));
    linearParams.add(zRotation.set("Z Rotation", 0, -180, 180));
    linearParams.add(mirrorX.set("Mirror X", false));
    linearParams.add(mirrorY.set("Mirror Y", false));
    
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
    int k = getKeyPressed();
    if (k == 's') linearAutoSlide = !linearAutoSlide;
    if (linearAutoSlide) linearSlide -= linearAutoSlideStep;
    
    // update manual controls
    if (k == OF_KEY_UP) linearScale *= 1.1;
    if (k == OF_KEY_DOWN) linearScale *= 0.9;
    if (k == OF_KEY_RIGHT) {
        if (linearAutoSlide) {
            linearAutoSlideStep -= 1;
        } else {
            linearSlide += linearSlideStep * 1/linearScale;
        }
    }
    if (k == OF_KEY_LEFT) {
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
    ofTranslate(ofGetWidth() / 2 + xTranslation, ofGetHeight() / 2 + yTranslation);
    
    ofRotateY(yRotation);
    ofRotateX(xRotation);
    ofRotateZ(zRotation);
    
    ofTranslate(0, 0, linearSlideLerp * linearScale);
    
    ofScale(1 - 2 * mirrorX, 1 - 2 * mirrorY);
    ofScale(linearScale, linearScale, linearScale);
    
    linearMesh.drawVertices();
    ofPopMatrix();
    
}

// ---------------------------------------------------------------------









//
//
//
//
//// fill mesh with a spherical capture
//// scans are spaced out according to the speed of the rotations (degrees/sec; default = 225/64) and the timestamp of each step
//// each period is 180 degrees since we're recording both sides of the lidar every scan
//void urgDisplay::fillPointMeshTXYSpherical(float speed, float period, bool bClockwise, float startingPeriod, float numPeriods, float alignmentAngle) {
//    
//    // clear the mesh
//    pointMesh.clear();
//    
//    // if either variable is -1, set it to default or max
//    if (startingPeriod == -1.) startingPeriod = 0;
//    if (numPeriods == -1.) numPeriods = 99999.;
//    
//    // ------------------------------------------
//    // ------- FIND INTERVAL OF INTEREST --------
//    // ------------------------------------------
//    
//    // set time zero
//    float timeZero;
//    
//    // interval of interest of the scans
//    int startIndex = -1;
//    int endIndex = -1;
//    
//    // first, find the starting points and ending points for the sphere
//    for (int i = 0; i < nScans; i++) {
//        
//        // find the current time
//        float timeNow = csv.getFloat(i, 0) / 1000.; // in seconds
//        
//        // check if this scan is the start of the interval of interest so long as it has not yet been found
//        if (startIndex == -1) {
//            if ((timeNow * speed) >= (startingPeriod * period)) {
//                startIndex = i;
//                timeZero = timeNow;
//            }
//        }
//        
//        // check if it's the end of the interval
//        if (startIndex != -1) {
//            if ((timeNow * speed) >= ((startingPeriod + numPeriods) * period)) {
//                endIndex = i;
//                break;
//            }
//        }
//    }
//    
//    // if endIndex wasn't assigned, assign it to the total number of scans
//    if (endIndex == -1) endIndex = nScans;
//    
//    // ------------------------------------------
//    // ---------- FILL THE POINT MESH -----------
//    // ------------------------------------------
//    
//    // for every scan within the interval, add it to the mesh
//    for (int i = startIndex; i < endIndex; i++) {
//        
//        // get the current time
//        float timeNow = csv.getFloat(i, 0) / 1000. - timeZero;
//        
//        // put all the points in a scan in the mesh
//        for (int j = minIndex; j < maxIndex; j++) { // NOTE: I've been doing this wrong (i.e. wrong order of operations... this is why there are so many points at zero, zero)... it's right here now
//            
//            // find the x and y coordinates
//            float px = csv.getFloat(i, 2 * j + 1);
//            float py = csv.getFloat(i, 2 * j + 2);
//            
//            // find the vector to this point
//            ofVec3f thisPoint(px, py, 0.);
//            
//            
//            
//            // remove points too close
//            double sqDist = thisPoint.distanceSquared(ofVec3f(0.,0.,0.));
//            
////            cout << sqDist << endl;
//
//            if (sqDist < minSqDist2Cam) continue;
//
//            
////            if (px < 100. && py < 100.) continue;
//            
//            // rotate this point 90 degrees about the z axis to orient it upwards
//            thisPoint.rotate(90., ofVec3f(0., 0., 1.));
//            
//            // apply the alignment angle stretch or compression to realign the two chuncks
//            float alignmentFactor = (float)j / 682. * alignmentAngle;
//            thisPoint.rotate(alignmentFactor, ofVec3f(0., 0., 1.));
//            
//            // rotate the point about the y axis an amount proportional to the elapsed time and the speed
//            float rotationAmt = timeNow * speed;
//            // if clockwise is true, rotate in negative direction
//            if (bClockwise) rotationAmt *= -1.;
//            thisPoint.rotate(rotationAmt, ofVec3f(0., 1., 0.));
//            
//            // add the point to the mesh with a color
//            pointMesh.addVertex(thisPoint);
//            pointMesh.addColor(ofFloatColor(1.));
//        }
//    }
//}
//
//// ---------------------------------------------------------------------
//
//// graphs 360 degree panorama
//// slide is the rotation here
//void urgDisplay::drawPointMeshSpherical(float scale, float slide) {
//    
//    easyCam.begin();
//    
//    ofPushMatrix();
//    
//    ofScale(scale, scale, scale);
//    
//    ofRotateY(slide / 80.);
//
//    pointMesh.drawVertices();
//    
//    ofPopMatrix();
//    
//    easyCam.end();
//    
//}
//
// ---------------------------------------------------------------------

void urgDisplay::setKeyPressed(int key_) {
    key = key_;
}

int urgDisplay::getKeyPressed() {
    int tempKey = key;
    key = 0;
    return tempKey;
}

