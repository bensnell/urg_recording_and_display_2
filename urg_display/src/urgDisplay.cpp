//
//  urgDisplay.cpp
//  urg_capture_display
//
//  Created by Ben Snell on 11/16/15.
//
//

#include "urgDisplay.h"

void urgDisplay::setupPointMesh() {
    
    // set mode to points
    pointMesh.setMode(OF_PRIMITIVE_POINTS);
    pointMesh.enableColors(); // must call if using colors

}

// ---------------------------------------------------------------------

void urgDisplay::setupTriangleMesh() {
    
    triMesh.setMode(OF_PRIMITIVE_TRIANGLES);
    triMesh.enableColors(); // must call if using colors
    triMesh.enableIndices();
}

// ---------------------------------------------------------------------

void urgDisplay::setMeshParameters(int zScale_, int minIndex_, int maxIndex_, double minSqDist2Cam_, double maxSqTriLength_) {
    
    zScale = zScale_;
    minIndex = minIndex_;
    maxIndex = maxIndex_;
    minSqDist2Cam = minSqDist2Cam_;
    maxSqTriLength = maxSqTriLength_;
    
}

// ---------------------------------------------------------------------

void urgDisplay::loadData(string folderName_, int fileNumber_) {
    
    csv.loadFile(getFilePath(folderName_, fileNumber_));
    
    nScans = csv.numRows;
    
    // Note: csv number of rows doesn't work
}

// ---------------------------------------------------------------------

// return the file path of the requested file in the data directory
string urgDisplay::getFilePath(string folder_name, int file_number) {
    
    // path to folder containing info
    string folderPath = ofToDataPath(folder_name);
    
    // open path to directory
    ofDirectory folderDir(folderPath);
    cout << "In folder " << folder_name;
    
    // populate directory with files
    folderDir.listDir();
    
    // get the specified file in the directory
    fileName = folderDir.getName(file_number);
    cout << " returning file " << fileName << endl;
    
    // return the path to the file
    filePath = folderDir.getPath(file_number);
    return filePath;
    
    /*
     Soutside Alleys, 11/7/2015
     setScanParams(100)
     {x0, y0, z0, x1, y1, z1, ...}
     0   not much
     1
     2
     3
     4
     5
     6
     7   YES: garage, tree, storefront, etc.
     8   bridge
     
     Carson St Sidewalk, 11/8/2015
     setScanParams(100, 100, 682, 70000., 100000.)
     {x0, y0, z0, x1, y1, z1, ...}
     0   nothing
     1   nothing
     2   nothing
     3   nothing
     4   nothing
     5   nothing
     6   nothing
     7   people, tree, open, some buildings
     8   people, cars
     9   crowd of people, zScale = 20
     10  two people sitting on sidewalk
     11  small segment of buildings and people
     12  some buildings
     13  some buildings
     14  some buildings, group of people
     15  storefronts, some people, hydrants
     16  buildings, people outlines
     17  long stretch of sidewalk, buildings, outlines of people
     18  lots of people, gets really close
     19  LOTS OF PEOPLE, storefronts, really rich
     20  buildings, neat storefronts
     21
     
     Carson St Sidewalk, 11/15/2015
     {yaw, pitch, roll, x0, y0, x1, y1, ...}
     0  magnometer wasn't working; looks like winding snake
     
     CFA Entrance, 11/17/2015
     {time, x0, y0 x1, y1, ...}
     21 some people
     23 
     
     Entrances, 11/17/2015
     0  Studio for Creative Inquiry, Golan
     4  long hallway, lots of people, sparse
     5
     
     */
}

// ---------------------------------------------------------------------

void urgDisplay::fillPointMeshXYZ() {
    
    pointMesh.clear();
    
    // for using the manhattan distance instead:
    //    double minManDist2Cam = pow(minSqDist2Cam/2, 1/2);
    
    // POINT MESH V1
    for (int i = 0; i < nScans; i++) {
        if (i == 0) countZero = zScale * csv.getFloat(i, 2);
        for (int j = minIndex; j < maxIndex; j++) {
            
            float px = csv.getFloat(i, 3 * j);
            float py = csv.getFloat(i, 3 * j + 1);
            
            double sqDist = ofVec3f(px, py, 0.).distanceSquared(ofVec3f(0.,0.,0.));
            if (sqDist < minSqDist2Cam) continue;
            
//            if (abs(px) < minManDist2Cam && abs(py) < minManDist2Cam) continue; // remove points close to scanner
            
            float pz = zScale/10 * csv.getFloat(i, 3 * j + 2) - countZero;
            
            pointMesh.addVertex(ofVec3f(px, py, pz));
            pointMesh.addColor(ofFloatColor(1.,1.,1.));
        }
    }
    
    countZero = 0;
}

// ---------------------------------------------------------------------

void urgDisplay::fillTriangleMeshXYZ() {
    
    triMesh.clear();
    
    // TRIANGLE MESH
    // loop through all vertices to add them
    // then add the indices to a triangle if they meet the requirements
    for (int i = 0; i < nScans; i++) {
        
        // get start count in file (time zero)
        if (i == 0) countZero = zScale * csv.getFloat(i, 2);
        
        for (int j = minIndex; j < maxIndex; j++) {     // 2178/3=726
            
            // add all vertices (don't discriminate at this step)
            float px = csv.getFloat(i, 3 * j);
            float py = csv.getFloat(i, 3 * j + 1);
            float pz = zScale/10 * csv.getFloat(i, 3 * j + 2) - countZero;
            triMesh.addVertex(ofVec3f(px, py, pz));
            triMesh.addColor(ofFloatColor(1., 1., 1.));
            
            // if it's the first row, move on (nothing to compare to)
            if (i == 0) continue;
            
            // if it's outside a certain bounding angle (index) of the lidar range, don't include it
            if (j < minIndex || j >= maxIndex) continue;
            
            // compare this scan with the previous one
            unsigned int indexA = i * 726 + j;
            unsigned int indexB = (i - 1) * 726 + j;
            unsigned int indexC = indexB + 1;
            unsigned int indexD = indexA - 1;
            
            // BUT hold on! If this point is within a certain (squared) distance to the lidar, don't bother making a triangle here
            ofVec3f vertA = triMesh.getVertex(indexA);
            double distToOrigin = vertA.distanceSquared(ofVec3f(0.,0.,vertA.z));
            if (distToOrigin < minSqDist2Cam) continue;
            
            // get the vertices that correpsond to these indices
            ofVec3f vertB = triMesh.getVertex(indexB);
            ofVec3f vertC = triMesh.getVertex(indexC);
            ofVec3f vertD = triMesh.getVertex(indexD);
            
            // calculate distances AB, BC, AD
            double distAB = vertA.distanceSquared(vertB);
            double distBC = vertB.distanceSquared(vertC);
            double distAD = vertA.distanceSquared(vertD);
            
            // if distAB is too large, don't make any triangles
            if (distAB > maxSqTriLength) continue;
            
            // possible triangle A for all j except last
            //             indexC
            //              /|
            //             / |
            //            /  |
            //    indexA /___| indexB
            if (j < 726 - 1) {
                // if the squared distance is large, don't bother making the triangle
                if (distBC < maxSqTriLength) {
                    triMesh.addIndex(indexA);
                    triMesh.addIndex(indexB);
                    triMesh.addIndex(indexC);
                }
            }
            
            // possible triangle B for all j except first
            //    indexA  ____ indexB
            //           |   /
            //           |  /
            //           | /
            //           |/
            //          indexD
            if (j > 0) {
                // if the squared distance AD is large, don't bother making the triangle
                if (distAD < maxSqTriLength) {
                    triMesh.addIndex(indexA);
                    triMesh.addIndex(indexB);
                    triMesh.addIndex(indexD);
                }
            }
        }
    }
    
    countZero = 0;
}

// ---------------------------------------------------------------------

void urgDisplay::fillPointMeshTWPRXY(float speed_, float alignYaw_, float alignPitch_, float alignRoll_, bool bCorrectGait) {
    
    speed = speed_;
    alignYaw = alignYaw_;
    alignPitch = alignPitch_;
    alignRoll = alignRoll_;
    
    pointMesh.clear();
    
    int startIndex = 0;
    int endIndex = nScans;
    
    // starting orientation, location and time
    ofVec3f lastPosition(0., 0., 0.);
    float yawZero = fmod(csv.getFloat(startIndex, 1) + 360., 360.); // put in range {0, 360}
    ofVec3f lastDirection(-1., 0., 0.); // oriention of the IMU is to the left (negative x axis)
    float timeZero = csv.getFloat(startIndex, 0) / 1000.;
    float lastTime = timeZero;
    
    // add every scan (cross section) to the mesh
    for (int i = startIndex; i < endIndex; i++) {
        
        // get the current location of scan by projecting the last scan direction with a magnitude of the last time interval from the last scan origin
        float thisTime = csv.getFloat(i, 0) / 1000.; // in seconds
        float distanceTraveled = (thisTime - lastTime) * speed;
        ofVec3f changeInPosition = lastDirection.getScaled(distanceTraveled);
        ofVec3f thisPosition = lastPosition + changeInPosition;
        
        // get current orientation of scan with respect to the starting orientation
        float thisYaw = fmod(csv.getFloat(i, 1) + 720. - yawZero, 360.);
        ofVec3f thisDirection = ofVec3f(-1., 0., 0.).rotate(-thisYaw, ofVec3f(0., 1., 0.));
        
        // for every point in the scan, rotate it accordingly, then translate it and add it to the mesh
        for (int j = minIndex + 4; j < maxIndex + 4; j++) {
            // get the initial (recorded) coordinates
            float px = csv.getFloat(i, 2 * j);
            float py = csv.getFloat(i, 2 * j + 1);
            ofVec3f recordedPosition(px, py, 0.);
            
            // cull out coorinates that are too close to the camera
            // use manhattan distance to speed up processing
//            double sqDist = recordedPosition.distanceSquared(ofVec3f(0.,0.,0.));
//            if (sqDist < minSqDist2Cam) continue;
            
            // first rotate so it's perpendicular to thisDirection (align virtual lidar scanner to coorindate system of razor IMU)
            ofVec3f alignedPosition = recordedPosition;
            alignedPosition.rotate(-alignRoll, 0., 0.);     // first roll (rotate right)
            alignedPosition.rotate(0., 0., -alignPitch);    // then pitch (pull back)
            alignedPosition.rotate(0., -alignYaw, 0.);      // finally yaw (turn clockwise)
            
            ofVec3f orientedPosition = alignedPosition;
            // optionally, correct for gait here (recorded pitch and roll)
            if (bCorrectGait) {
                float thisRoll = csv.getFloat(i, 3);
                orientedPosition.rotate(-thisRoll, 0., 0.);
                float thisPitch = csv.getFloat(i, 2);
                orientedPosition.rotate(0., 0., -thisPitch);
            }
            
            // orient position to thisDirection
            orientedPosition.rotate(0., -thisYaw, 0.);
            
            // move point to actual location
            ofVec3f translatedPosition = orientedPosition + thisPosition;
            
            // add translated position to the mesh
            pointMesh.addVertex(translatedPosition);
            pointMesh.addColor(ofFloatColor(1., 1., 1.));
        }
        
        lastTime = thisTime;
        lastDirection = thisDirection;
        lastPosition = thisPosition;
    }
    
}

// ---------------------------------------------------------------------

void urgDisplay::fillPointMeshTXY(int startScan, int endScan, bool bTimeDependent) {
    
    pointMesh.clear();
    
    // for using the manhattan distance instead:
    //    double minManDist2Cam = pow(minSqDist2Cam/2, 1/2);
    
    // if startScan or endScan is -1, use the min or max value, respectively
    if (startScan == -1) startScan = 0;
    if (endScan == -1) endScan = nScans;
    // check to make sure the endScan, if not -1, isn't greater than nScans
    if (endScan > nScans) {
        endScan = nScans;
        cout << "endScan is too high. defaulting to total number of scans" << endl;
    }
    
    float timeZero;
    
    // POINT MESH V1
    for (int i = startScan; i < endScan; i++) {
        
        // if rendering time-dependent, find the current time
        float timeNow;
        if (bTimeDependent) {
            if (i == startScan) {
                timeZero = csv.getFloat(i, 0) / 1000.;
                timeNow = 0.;
            } else {
                timeNow = csv.getFloat(i, 0) / 1000. - timeZero;
            }
        }
        
        for (int j = minIndex + 1; j < maxIndex + 1; j++) {
            
            float px = csv.getFloat(i, 2 * j);
            float py = csv.getFloat(i, 2 * j + 1);
            
            double sqDist = ofVec3f(px, py, 0.).distanceSquared(ofVec3f(0.,0.,0.));
            if (sqDist < minSqDist2Cam) continue;
            
//            if (abs(px) < minManDist2Cam && abs(py) < minManDist2Cam) continue; // remove points close to scanner
            
            // if time dependent, graph pz proportional to elapsed time; otherwise, graph pz with constant spacing
            float pz;
            if (bTimeDependent) {
                pz = timeNow * zScale;
            } else {
                // each i is 100 ms
                pz = (i - startScan) / 10. * zScale;
            }
            
            pointMesh.addVertex(ofVec3f(px, py, pz));
            pointMesh.addColor(ofFloatColor(1.,1.,1.));
        }
    }
    
    countZero = 0;
}

// ---------------------------------------------------------------------

// fill point mesh with slighly transparent points ouside of given bounds
void urgDisplay::fillPointMeshTXYGhosted(int xMin, int xMax, int yMin, int yMax, float transparency, Boolean timeDependent) {
    
    // clear the previous mesh, if one exists
    pointMesh.clear();
    
    // stores starting time
    float timeZero;
    
    // for every scan, add it to the mesh
    for (int i = 0; i < nScans; i++) {
        
        float timeNow;
        if (timeDependent) {
            // find the time now (in seconds) and if it's the first scan, time zero
            if (i == 0) {
                timeZero = csv.getFloat(i, 0) / 1000.;
                timeNow = 0.;
            } else {
                timeNow = csv.getFloat(i, 0) / 1000. - timeZero;
            }
        }
        
        // for culling data points that are too close
        float prevx = 0.;
        float prevy = 0.;
        
        // for every point in the scan,
        for (int j = minIndex + 1; j < maxIndex + 1; j++) {
            
            // get the x and y location of the point from the csv
            float px = csv.getFloat(i, 2 * j);
            float py = csv.getFloat(i, 2 * j + 1);
            
            // cull out "free radials" (extra readings along horizon)
            if (px < 5 && px > -5 && py < yMax) continue;
            
            // if the reading has a y around or less than 0, cut it out
            if (py < 10) continue;
            
            // remove points within a certain distance of the scan if the reference distance isn't zero
            if (minSqDist2Cam != 0.) {
                double sqDist = ofVec3f(px, py, 0.).distanceSquared(ofVec3f(0.,0.,0.));
                if (sqDist < minSqDist2Cam) continue;
            }
            
            // find the color for this point, depending on the boundaries passed in (this will make it ghosted)
            ofFloatColor thisColor;
            if (px < xMin || px > xMax || py < yMin || py > yMax) {
                // make transparent (shades of white)
                thisColor.set(1., transparency);
                
                // OPTIONAL RESAMPLE
                // cull out points that are too close to each other (only do this to the outer ghosted points)
                int minProximity = 2;
                if (abs(prevx - px) < minProximity || abs(prevy - py) < minProximity) {
                    continue; // cull it
                } else {
                    prevx = px;
                    prevy = py;
                }
                
            } else {
                // make opaque (white)
                thisColor.set(1.);
            }
            
            // find the current z value based on the elapsed time since timeZero or on the counter and estimation of time (1/10 sec per reading)
            float pz;
            if (timeDependent) {
                pz = timeNow * zScale;
            } else {
                pz = i * 0.1 * zScale;
            }
            
            pointMesh.addVertex(ofVec3f(px, py, pz));
            pointMesh.addColor(thisColor);
            
            // OPTIONAL MIRRORING
            // mirror opposite side and keep running tally of the best points to mirror
            if (py > yMax && py < (yMax + 500)) {
                mirrorPoints[j] = ofVec3f(px, yMin - (py - yMax), pz); // new point
            }
            // if a point exists at j in array, add it to the mesh with a little randomness
            if ((mirrorPoints[j].x + mirrorPoints[j].y + mirrorPoints[j].z) != 0.) {
                pointMesh.addVertex(ofVec3f(mirrorPoints[j].x - 50 + ofRandom(-5., 5.), mirrorPoints[j].y + ofRandom(-5., 5.), pz));
                pointMesh.addColor(ofFloatColor(1., transparency));
            }
            
        }
    }
}

// ---------------------------------------------------------------------

// fill point mesh with slighly transparent points ouside of given bounds
// USES TWO MESHES INSTEAD OF ONE
void urgDisplay::fillPointMeshTXYGhostedDouble(int startScan, int endScan, int xMin, int xMax, int yMin, int yMax, float transparency, bool timeDependent, bool bResample) {
    
    // clear the previous mesh, if one exists
    pointMeshOpaque.clear();
    pointMeshTransparent.clear();
    
    // if startScan or endScan is -1, use the min or max value, respectively
    if (startScan == -1) startScan = 0;
    if (endScan == -1) endScan = nScans;
    // check to make sure the endScan, if not -1, isn't greater than nScans
    if (endScan > nScans) {
        endScan = nScans;
        cout << "endScan is too high. defaulting to total number of scans" << endl;
    }
    
    // stores starting time
    float timeZero;
    
    // for every scan, add it to the mesh
    for (int i = startScan; i < endScan; i++) {
        
        float timeNow;
        if (timeDependent) {
            // find the time now (in seconds) and if it's the first scan, time zero
            if (i == 0) {
                timeZero = csv.getFloat(i, 0) / 1000.;
                timeNow = 0.;
            } else {
                timeNow = csv.getFloat(i, 0) / 1000. - timeZero;
            }
        }
        
        // for culling data points that are too close outside of bounds
        float prevx = 0.;
        float prevy = 0.;
        
        // for every point in the scan,
        for (int j = minIndex + 1; j < maxIndex + 1; j++) {
            
            // get the x and y location of the point from the csv
            float px = csv.getFloat(i, 2 * j);
            float py = csv.getFloat(i, 2 * j + 1);
            
            // cull out "free radials" (extra readings along horizon)
            if (px < 7 && px > -7 && py < (yMax + 100)) continue;
            if (px < 7 && px > -7 && py > (yMax + 140)) continue;
            
            // if the reading has a y around or less than 0, cut it out
            if (py < 10) continue; // optional
            
            // remove points within a certain distance of the scan if the reference distance isn't zero
            if (minSqDist2Cam != 0.) {
                double sqDist = ofVec3f(px, py, 0.).distanceSquared(ofVec3f(0.,0.,0.));
                if (sqDist < minSqDist2Cam) continue;
            }
            
            // find the current z value based on the elapsed time since timeZero or on the counter and estimation of time (1/10 sec per reading)
            float pz;
            if (timeDependent) {
                pz = timeNow * zScale;
            } else {
                pz = i * 0.1 * zScale;
            }
            
            // find the color for this point, depending on the boundaries passed in (this will make it ghosted)
            ofFloatColor thisColor;
            if (px < xMin || px > xMax || py < yMin || py > yMax) {
                
                // OPTIONAL RESAMPLE
                // cull out points that are too close to each other (only do this to the outer ghosted points)
                if (bResample) {
                    int minProximity = 2;
                    if (abs(prevx - px) < minProximity || abs(prevy - py) < minProximity) {
                        continue; // cull it
                    } else {
                        prevx = px;
                        prevy = py;
                    }
                }
                
                // make transparent (shades of white)
                thisColor.set(1., transparency);
                
                pointMeshTransparent.addVertex(ofVec3f(px, py, pz));
                pointMeshTransparent.addColor(thisColor);
                
            } else {
                // make opaque (white)
                thisColor.set(1.);
                pointMeshOpaque.addVertex(ofVec3f(px, py, pz));
                pointMeshOpaque.addColor(thisColor);
            }
            
            // OP MIRRORING (for carnegie museum)
//            if (px < xMin) {
//                mirrorPoints[j] = ofVec3f(px, yMax / 2 + yMax / 2 - py, pz);
//            }
            
            // OPTIONAL MIRRORING -- only for transparent mesh (for wean hallway)
            // mirror opposite side and keep running tally of the best points to mirror
//            if (py > yMax && py < (yMax + 500)) {
//                mirrorPoints[j] = ofVec3f(px, yMin - (py - yMax), pz); // new point
//            }
            // if a point exists at j in array, add it to the mesh with a little randomness
//            if ((mirrorPoints[j].x + mirrorPoints[j].y + mirrorPoints[j].z) != 0.) {
//                pointMeshTransparent.addVertex(ofVec3f(mirrorPoints[j].x - 50 + ofRandom(-5., 5.), mirrorPoints[j].y + ofRandom(-5., 5.), pz));
//                pointMeshTransparent.addColor(ofFloatColor(1., transparency));
//            }
            
            
            
        }
    }
}

// ---------------------------------------------------------------------

// buffer size is the max number of scans without people in them at the ends of the capture or on either side of each person
void urgDisplay::resampleGhostedPointMeshes(int startScan, int endScan, int xMin, int xMax, int yMin, int yMax, float transparency, bool bResample, string exportName, int bufferSizeEnds, int bufferSizePeople) {
    
    // clear the previous mesh, if one exists
    pointMeshOpaque.clear();
    pointMeshTransparent.clear();
    
    // if startScan or endScan is -1, use the min or max value, respectively
    if (startScan == -1) startScan = 0;
    if (endScan == -1) endScan = nScans;
    // check to make sure the endScan, if not -1, isn't greater than nScans
    if (endScan > nScans) {
        endScan = nScans;
        cout << "endScan is too high. defaulting to total number of scans" << endl;
    }
    
    // keeps count of scans added to meshes (use to set z value)
    unsigned long counter = 0;
    
//    cout << "there are this many people: " << people.size() << " and this many scans: " << nScans << endl;
    
    // For every person
    for (long i = 0; i < people.size(); i++) {
        
//        cout << "now person " << i << " with start " << people[i].scanStart << " and length " << people[i].scanLength << endl;
        
        long thisStart = MAX(0, people[i].scanStart - bufferSizePeople);
        if (i == 0) thisStart = MAX(0, thisStart - bufferSizeEnds);
        
        long thisEnd = MIN(nScans, people[i].scanStart + people[i].scanLength + bufferSizePeople);
        if (i == people.size() - 1) thisEnd = MIN(nScans, thisEnd + bufferSizeEnds);
        
//        cout << "plan to use scans " << thisStart << " to " << thisEnd << endl;
        
        // For every scan in that person
        for (long j = thisStart; j < thisEnd; j++) { // change to long
            
            // skip the rest of a person's scans if at any point they overlap with the next person's scans
            if (i != people.size() - 1) {
                if (j >= MAX(0, people[i + 1].scanStart - bufferSizePeople)) {
                    cout << "stop at " << j << " for person # " << (i) << endl;
                    break;
                }
            }
            
            // ------------------------
            // --- NOW ADD THE SCAN ---
            // ------------------------
            
            // for culling data points that are too close outside of bounds
            float prevx = 0.;
            float prevy = 0.;
            
            // for every point in the scan, add it to the mesh
            for (int k = minIndex + 1; k < maxIndex + 1; k++) {
                
                // get the x and y location of the point from the csv
                float px = csv.getFloat(j, 2 * k); // CSV ONLY ACCEPTS INTS HERE!
                float py = csv.getFloat(j, 2 * k + 1);
                
                // cull out "free radials" (extra readings along horizon)
                if (px < 5 && px > -5 && py < (yMax + 105)) continue;
                if (px < 5 && px > -5 && py > (yMax + 135)) continue;
                
                // if the reading has a y around or less than 0, cut it out
                if (py < 10) continue; // optional
                
                // remove points within a certain distance of the scan if the reference distance isn't zero
                if (minSqDist2Cam != 0.) {
                    double sqDist = ofVec3f(px, py, 0.).distanceSquared(ofVec3f(0.,0.,0.));
                    if (sqDist < minSqDist2Cam) continue;
                }
                
                // find the current z value based on the number of scans added to the meshes
                float pz = counter * 0.1 * zScale;
                
                // find the color for this point, depending on the boundaries passed in (this will make it ghosted)
                ofFloatColor thisColor;
                if (px < xMin || px > xMax || py < yMin || py > yMax) {
                    
                    // OPTIONAL RESAMPLE
                    // cull out points that are too close to each other (only do this to the outer ghosted points)
                    if (bResample) {
                        int minProximity = 5;
                        if (abs(prevx - px) < minProximity || abs(prevy - py) < minProximity) {
                            continue; // cull it
                        } else {
                            prevx = px;
                            prevy = py;
                        }
                    }
                    
                    // make transparent (shades of white)
                    thisColor.set(1., transparency);
                    
                    pointMeshTransparent.addVertex(ofVec3f(px, py, pz));
                    pointMeshTransparent.addColor(thisColor);
                    
                } else {
                    // make opaque (white)
                    thisColor.set(1.);
                    pointMeshOpaque.addVertex(ofVec3f(px, py, pz));
                    pointMeshOpaque.addColor(thisColor);
                }
            }
            
            counter++;
        }
    }
    
    // save the meshes to file
        pointMeshOpaque.save(exportName + "_opaque.ply");
        pointMeshTransparent.save(exportName + "_transparent.ply");
}

// ---------------------------------------------------------------------

// works for XYZ and TXY
// when mirror is true, mirrors across x
void urgDisplay::drawPointMeshLinear(float scale, float slide, float zRotation, bool bMirror) {
    
    int mouseX = ofGetAppPtr()->mouseX;
    int mouseY = ofGetAppPtr()->mouseY;
    
    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
    ofRotateY(ofMap(mouseX, 0, ofGetWidth(), -90, 90));
    ofRotateX(ofMap(mouseY, 0, ofGetHeight(), -90, 90));
    if (bMirror) ofScale(-1, 1, 1);
    ofRotateZ(zRotation);
    ofTranslate(0., 0., slide * scale);
    ofScale(scale, scale, scale);
//        ofSetColor(255, 200, 200);
//        ofCircle(0, 0, 100);
    pointMesh.drawVertices();
    ofPopMatrix();
}

// ---------------------------------------------------------------------

void urgDisplay::drawTriangleMeshXYZ(float scale, float slide) {
    
    int mouseX = ofGetAppPtr()->mouseX;
    int mouseY = ofGetAppPtr()->mouseY;
    
    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
    ofRotateY(ofMap(mouseX, 0, ofGetWidth(), -90, 90));
    ofRotateX(ofMap(mouseY, 0, ofGetHeight(), -90, 90));
    ofRotateZ(180);
    ofTranslate(0., 0., slide * scale);
    ofScale(scale, scale, scale);
    //    ofSetColor(255, 200, 200);
    //    ofCircle(0, 0, 100);
    triMesh.drawFaces();
    ofPopMatrix();
}

// ---------------------------------------------------------------------

void urgDisplay::drawPointMeshTWPRXY(float scale, float slide) {
    
//    int mouseX = ofGetAppPtr()->mouseX;
//    int mouseY = ofGetAppPtr()->mouseY;
    
    easyCam.begin();
    ofPushMatrix();
//    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
//    ofRotateY(ofMap(mouseX, 0, ofGetWidth(), -90, 90));
//    ofRotateX(ofMap(mouseY, 0, ofGetHeight(), -90, 90));
    ofTranslate(- slide * scale, 0., 0.);
    ofScale(scale, scale, scale);
    //        ofSetColor(255, 200, 200);
    //        ofCircle(0, 0, 100);
    pointMesh.drawVertices();
    ofPopMatrix();
    easyCam.end();
}

// ---------------------------------------------------------------------

void urgDisplay::drawOrthoGhostedTimeline(float xRotation, float yRotation, float zRotation, float xTranslation, float yTranslation, float scale, float slide) {

    int key;
    ofGetAppPtr()->keyPressed(key); // DOES NOT WORK... just call getKepPressed();
    
    // set the parameters and position of the camera
    camera.setFarClip(500);
    camera.setNearClip(0);
    camera.enableOrtho();
    
    ofPushMatrix();

    // begin the camera
    camera.begin();
    
    // translate origin to center of screen
    ofTranslate(ofGetWidth() / 2 + xTranslation, ofGetHeight() / 2 + yTranslation, 0);
    
    // rotate the timeline in the z direction if it's not naturally right side up
    ofRotateZ(zRotation);
    
    // rotate the timeline so it opens up to the bottom left
    ofRotateX(xRotation);
    ofRotateY(yRotation);
    
    // move to the next person in the timeline (using the right or left key pressed)
    // xxx
     ofTranslate(0., 0., slide / 10);
    
    // draw the labels?
    
    ofScale(scale, scale, scale);
    
    // draw the vertices
    pointMesh.drawVertices();
    
    // end the camera
    camera.end();
    
    ofPopMatrix();
}

// ---------------------------------------------------------------------

// draw pair of transparent and opaque meshes
void urgDisplay::drawGhostedPointMeshes(float scale, float slide, float xRotation, float yRotation, float zRotation, bool bOrtho, bool bRotateControl, bool bSlideControl, bool bFlipX, bool bFlipY, float xTranslation, float yTranslation, bool bDrawPeopleLocations) {
    
    if (bOrtho) {
        camera.setFarClip(500);
        camera.setNearClip(0);
        camera.enableOrtho(); // what happens if ortho comes before setting the clip?
    }
    
    int mouseX = ofGetAppPtr()->mouseX;
    int mouseY = ofGetAppPtr()->mouseY;
    
    if (bOrtho) camera.begin();
    
    ofPushMatrix();
    
    ofTranslate(ofGetWidth() / 2 + xTranslation, ofGetHeight() / 2 + yTranslation, 0.);
    
    ofRotateX(xRotation);
    if (bRotateControl) ofRotateX(ofMap(mouseY, 0, ofGetHeight(), -90, 90));
    ofRotateY(yRotation);
    if (bRotateControl) ofRotateY(ofMap(mouseX, 0, ofGetWidth(), -90, 90));
    
    ofRotateZ(zRotation);
    
//    if (getKeyPressed() == 32) {
//        
//        // find the next person greater than where you are or are heading
//        while (destination > people[pplCount].startScan) {
//            pplCount++;
//            
//            // if you get to the last person, restart
//            if (pplCount == people.size()) {
//                pplCount = 0;
////                destination = people[pplCount].startScan;
//                break;
//            }
//        }
//        
//        // set the destination
//        destination = people[pplCount].startScan;
//    
//    } else {
//        
//        destination += 100;
//    }
//    
//    // if it slides past the last person, restart
//    if (destination > people[people.size()].startScan) {
//        pplCount = 0;
//        destination = people[pplCount].startScan;
//    }
//    
//    currentLocation = currentLocation + ((float)destination - (float)currentLocation) * 0.1;
    
//    currentLocation += 1.;
//    cout << currentLocation << endl;
//    if (currentLocation > people[people.size()].startScan) {
////        currentLocation = people[0].startScan;
//    }

    if (bSlideControl) {
        ofTranslate(0., 0., slide / 10);
    } else {
        currentLocation += 1.;
        ofTranslate(0., 0., -currentLocation * .2);
    }
    
    ofScale(1 - 2 * bFlipX, 1 - 2 * bFlipY); // flip in true directions
    ofScale(scale, scale, scale);
    
    pointMeshOpaque.drawVertices();
    pointMeshTransparent.drawVertices();
    
    // draw locations of people
    if (bDrawPeopleLocations) {
        for (int i = 0; i < people.size(); i++) {
            ofSetColor(255);
            
            ofCircle(ofVec3f(0., 0., people[i].startLocation), 50);
        }
    }
    
    ofPopMatrix();
    
    if (bOrtho) camera.end();
}

// ---------------------------------------------------------------------

//void urgDisplay::drawPeople() {
//    
//    // draw locations of people
//        for (int i = 0; i < people.size(); i++) {
//            ofSetColor(255);
//    
//            ofCircle(ofVec3f(0., 0., people[i].startScan), 50);
//        }
//}




/*

int mouseX = ofGetAppPtr()->mouseX;
int mouseY = ofGetAppPtr()->mouseY;

//    int key;
//    ofGetAppPtr()->keyPressed(key);

ofPushMatrix();

ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);

ofRotateY(-60);
ofRotateX(-10);
ofRotateZ(90);

urg.currentLocation = urg.currentLocation + ((float)urg.destination - (float)urg.currentLocation) * 0.1;
//    cout << moveAmount << endl;
ofTranslate(0., 0., -urg.currentLocation * .2);
ofScale(.2, .2, .2);

urg.pointMeshOpaque.drawVertices();
urg.pointMeshTransparent.drawVertices();

// draw locations of people
for (int i = 0; i < urg.people.size(); i++) {
    //        ofSetColor(255);
    
    //        ofCircle(ofVec3f(0., 0., urg.people[i].startScan), 50);
}

ofPopMatrix();

*/

//void urgDisplay::myKeyPressed(ofEventArgs & args) {
//    thisKey = key;
//}

// ---------------------------------------------------------------------

// not time dependent
void urgDisplay::findPeople(int startScan, int endScan, int xMin, int xMax, int yMin, int yMax, int minPointCount, int maxDistContinuity, bool bYHeight) {
    
    people.clear();
    
    // if startScan or endScan is -1, use the min or max value, respectively
    if (startScan == -1) startScan = 0;
    if (endScan == -1) endScan = nScans;
    // check to make sure the endScan, if not -1, isn't greater than nScans
    if (endScan > nScans) {
        endScan = nScans;
        cout << "endScan is too high. defaulting to total number of scans" << endl;
    }
    
    person runningPerson;   // last person captured, if still in the scan
    
    float avgWeight = 0.5;  // weight of running average
    
    // For each scan...
    for (int i = startScan; i < endScan; i++) {
        
        // -------------------------
        // ------ SAMPLE SCAN ------
        // -------------------------
        
        // Find the count and sum of y values for every point within the specified bounds of a scan
        float sumHeights = 0;     // sum of y values of points within bounds
        int nPts = 0;       // number of points within bounds
        for (int j = minIndex + 1; j < maxIndex + 1; j++) {
            
            // get x and y
            float px = csv.getFloat(i, 2 * j);
            float py = csv.getFloat(i, 2 * j + 1);
            
            // if the point is in the bounds...
            if (px > xMin && px < xMax && py > yMin && py < yMax) {
                
                // ...add it to the sum and increment nPts
                if (bYHeight) {
                    sumHeights += py;
                } else {
                    sumHeights += px;
                }
                nPts++;
            }
        }
        
        // -------------------------
        // ----- UPDATE PEOPLE -----
        // -------------------------
        
        // If the number of points is greater than the minPointCount, there is a person here
        if (nPts > minPointCount) {
            
            // calculate average height
            float avgHeight = sumHeights / (float)nPts;
            
            // if lastPerson is empty, create a new person (lastPerson)
            if (runningPerson.nDatums == 0) {
                
                // update running person
                runningPerson.nDatums += nPts;
                runningPerson.scanStart = i;
                runningPerson.scanLength++;
                runningPerson.startLocation = i * 0.1 * zScale; // scan index location
                runningPerson.runningAvgHeight = avgHeight;
            }
            
            // otherwise, if this person is close enough to the runningPerson, update runningPerson
            else if (abs(avgHeight - runningPerson.runningAvgHeight) < maxDistContinuity) {
                
                // update running person
                runningPerson.nDatums += nPts;
                runningPerson.scanLength++;
                runningPerson.runningAvgHeight = avgWeight * avgHeight + (1. - avgWeight) * runningPerson.runningAvgHeight;
            }
            
            // otherwise, this is a new person, so end the last person and create a new person
            else {
                
                // add running person to people vector
                people.push_back(runningPerson);
                
                // start a new running person
                runningPerson.nDatums = nPts;
                runningPerson.scanStart = i;
                runningPerson.scanLength = 1;
                runningPerson.startLocation = i * 0.1 * zScale;
                runningPerson.runningAvgHeight = avgHeight;
            }
        }
        
        // If the number of points in bounds is less than the minPointCount, there is no person here
        else {
            
            // if there's a running person, finish them
            if (runningPerson.nDatums > 0) {
                
                // add running person to people vector
                people.push_back(runningPerson);
                
                // clear running person
                runningPerson.nDatums = 0;
                runningPerson.scanStart = 0;
                runningPerson.scanLength = 0;
                runningPerson.startLocation = 0;
                runningPerson.runningAvgHeight = 0;
            }
            
            // otherwise, don't do anything
        }
        
    }
    
    // if running person is still in the last frame, finish them
    if (runningPerson.nDatums > 0) {
        
        people.push_back(runningPerson);
    }
    
    
    
    
    
//            // if the vector of temp people is empty, create a new person
//            if (tempPeople.empty()) {
//                
////                cout << "tempPeople empty.    now, it's: ";
//                
//                person tempPerson;
//                tempPerson.nDatums += nPts;
//                tempPerson.startScan = i * 0.1 * zScale; // scan index location
//                tempPerson.runningAvgY = avgWeight * avgY + (1. - avgWeight) * tempPerson.runningAvgY;
//                
//                // add to tempPeople vector from the beginning
//                tempPeople.insert(tempPeople.begin(), tempPerson);
//                
////                cout << tempPeople.size() << endl;
//                
//                // note: could find actual average using nDatums
//            }
//            // otherwise, check the tempPeople to see if this is a continuation of a person
//            else {
//                
////                cout << "TP not empty. It has size: " << tempPeople.size() << endl;
//                
//                for (int k = 0; k < tempPeople.size(); k++) {
//                    
//                    // find the avgY to compare to
//                    float referenceAvgY = tempPeople[k].runningAvgY;
//                    
//                    // if the avg y's are within the maxDistContinuity, it's the same person, so update this person's info
//                    if (abs(referenceAvgY - avgY) < maxDistContinuity) {
//                        
////                        cout << "close enough to consider the same person" << endl;
//                        
//                        tempPeople[k].nDatums += nPts;
//                        tempPeople[k].runningAvgY = avgWeight * avgY + (1. - avgWeight) * tempPeople[k].runningAvgY;
//                        
//                        // the avgY can't be for two people, so flag that this person has been updated so we can finish off the non-updated people
//                        tempPeople[k].updatedFlag = true;
//                        
////                        cout << "make flag of " << k << " true" << endl;
//                        
//                        // exit from the loop so we don't update any other people
//                        break;
//                    }
//                }
//                
////                cout << "here" << endl;
//                
//                // go through the updatedFlags of all tempPeople to either keep them or finish them, THEN and only then can you create a new person if the avgY's weren't close enough (foundMatch flag)
//                Boolean foundMatch = false;
//                for (int k = tempPeople.size() - 1; k >= 0; k--) { // iterate backwards to prevent deleting the same elements in vector
//                    
////                    cout << k << endl;
//                    
//                    // if person was udpated keep it
//                    if (tempPeople[k].updatedFlag) {
//                        tempPeople[k].updatedFlag = false; // give them another chance
//                        // mark that the avgY was matched with a person
//                        foundMatch = true;
//                        
////                        cout << "found a match. keep em" << endl;
//                        
//                    }
//                    // otherwise, finish them and add them to the people array
//                    else {
//                        people.push_back(tempPeople[k]);
//                        
////                        cout << "erase person " << k;
//                        
//                        // delete from temp people array
//                        tempPeople.erase(tempPeople.begin() + k);
//                        
////                        cout << ".   now TP is this long: " << tempPeople.size() << endl;
//                        
//                        
//                    }
//                }
//                
////                cout << "done checking the other temp people. TP is now this long: " << tempPeople.size() << endl;
//                
//                // if foundMatch is false, create a new person (in what is now an empty array)
//                if (!foundMatch) {
//                    
////                    cout << "didn't find a match in TP, so let's create a new person. now TP.size() = ";
//                    
//                    person tempPerson;
//                    tempPerson.nDatums += nPts;
//                    tempPerson.startScan = i * 0.1 * zScale; // scan index location
//                    tempPerson.runningAvgY = avgWeight * avgY + (1. - avgWeight) * tempPerson.runningAvgY;
//                    
//                    // add to tempPeople vector from the beginning
//                    // note: could push back here since the vector is empty
//                    tempPeople.insert(tempPeople.begin(), tempPerson);
//                    
////                    cout << tempPeople.size() << endl;
//                }
//            }
//        }
        
}

// ---------------------------------------------------------------------

void urgDisplay::loadGhostedPointMeshes(string pointMeshOpaqueName, string pointMeshTransparentName) {

    pointMeshOpaque.clear();
    pointMeshOpaque.load(pointMeshOpaqueName);
    
    pointMeshTransparent.clear();
    pointMeshTransparent.load(pointMeshTransparentName);
    
}


// ---------------------------------------------------------------------

void urgDisplay::setKeyPressed(int key) {
    thisKey = key;
}

int urgDisplay::getKeyPressed() {
    int tempKey = thisKey;
    thisKey = 0;
    return tempKey;
}

//void urgDisplay::update() {
//    
//    for(int i = 0; i < pointMesh.getNumColors(); i++) {
//        
//        ofVec3f v = pointMesh.getVertex(i);
//        
//        pointMesh.setColor(i, ofColor(   v.x  ,v.y, v.z));
//    }
//    
//}

// ---------------------------------------------------------------------

// fill mesh with a spherical capture
// scans are spaced out according to the speed of the rotations (degrees/sec; default = 225/64) and the timestamp of each step
// each period is 180 degrees since we're recording both sides of the lidar every scan
void urgDisplay::fillPointMeshTXYSpherical(float speed, float period, bool bClockwise, float startingPeriod, float numPeriods, float alignmentAngle) {
    
    // clear the mesh
    pointMesh.clear();
    
    // if either variable is -1, set it to default or max
    if (startingPeriod == -1.) startingPeriod = 0;
    if (numPeriods == -1.) numPeriods = 99999.;
    
    // ------------------------------------------
    // ------- FIND INTERVAL OF INTEREST --------
    // ------------------------------------------
    
    // set time zero
    float timeZero;
    
    // interval of interest of the scans
    int startIndex = -1;
    int endIndex = -1;
    
    // first, find the starting points and ending points for the sphere
    for (int i = 0; i < nScans; i++) {
        
        // find the current time
        float timeNow = csv.getFloat(i, 0) / 1000.; // in seconds
        
        // check if this scan is the start of the interval of interest so long as it has not yet been found
        if (startIndex == -1) {
            if ((timeNow * speed) >= (startingPeriod * period)) {
                startIndex = i;
                timeZero = timeNow;
            }
        }
        
        // check if it's the end of the interval
        if (startIndex != -1) {
            if ((timeNow * speed) >= ((startingPeriod + numPeriods) * period)) {
                endIndex = i;
                break;
            }
        }
    }
    
    // if endIndex wasn't assigned, assign it to the total number of scans
    if (endIndex == -1) endIndex = nScans;
    
    // ------------------------------------------
    // ---------- FILL THE POINT MESH -----------
    // ------------------------------------------
    
    // for every scan within the interval, add it to the mesh
    for (int i = startIndex; i < endIndex; i++) {
        
        // get the current time
        float timeNow = csv.getFloat(i, 0) / 1000. - timeZero;
        
        // put all the points in a scan in the mesh
        for (int j = minIndex; j < maxIndex; j++) { // NOTE: I've been doing this wrong (i.e. wrong order of operations... this is why there are so many points at zero, zero)... it's right here now
            
            // find the x and y coordinates
            float px = csv.getFloat(i, 2 * j + 1);
            float py = csv.getFloat(i, 2 * j + 2);
            
            // find the vector to this point
            ofVec3f thisPoint(px, py, 0.);
            
            
            
            // remove points too close
            double sqDist = thisPoint.distanceSquared(ofVec3f(0.,0.,0.));
            
//            cout << sqDist << endl;

            if (sqDist < minSqDist2Cam) continue;

            
//            if (px < 100. && py < 100.) continue;
            
            // rotate this point 90 degrees about the z axis to orient it upwards
            thisPoint.rotate(90., ofVec3f(0., 0., 1.));
            
            // apply the alignment angle stretch or compression to realign the two chuncks
            float alignmentFactor = (float)j / 682. * alignmentAngle;
            thisPoint.rotate(alignmentFactor, ofVec3f(0., 0., 1.));
            
            // rotate the point about the y axis an amount proportional to the elapsed time and the speed
            float rotationAmt = timeNow * speed;
            // if clockwise is true, rotate in negative direction
            if (bClockwise) rotationAmt *= -1.;
            thisPoint.rotate(rotationAmt, ofVec3f(0., 1., 0.));
            
            // add the point to the mesh with a color
            pointMesh.addVertex(thisPoint);
            pointMesh.addColor(ofFloatColor(1.));
        }
    }
}

// ---------------------------------------------------------------------

// graphs 360 degree panorama
// slide is the rotation here
void urgDisplay::drawPointMeshSpherical(float scale, float slide) {
    
    easyCam.begin();
    
    ofPushMatrix();
    
    ofScale(scale, scale, scale);
    
    ofRotateY(slide / 80.);

    pointMesh.drawVertices();
    
    ofPopMatrix();
    
    easyCam.end();
    
}


