//
//  urgDisplay.h
//  urg_capture_display
//
//  Created by Ben Snell on 11/16/15.
//
//

#ifndef __urg_capture_display__urgDisplay__
#define __urg_capture_display__urgDisplay__

#include "ofMain.h"

class urgDisplay {
    
public:
    
    urgDisplay();
    
    // ---------------------------
    // ------- LINEAR MESH -------
    // ---------------------------
    
    // holds linear mesh
    ofMesh linearMesh;
    
    // load data from a csv into a buffer in the following format
    //      time   x0  y0  x1  y1  x2  y2 ...
    void loadLinearData(string fileName);
    
    ofBuffer linearBuffer;
    unsigned long nLinearScans;
    
    // fill the linear mesh with points according to the following parameters
    void fillLinearMesh(int startScan = 0, int endScan = -1, int zScale = 300, int minIndex = 0, int maxIndex = 682, bool timeDependent = false, int cullDistance = 265, ofColor color = ofColor(255));
    /*  
        startScan       starting scan index to include in mesh
        endScan         ending scan index to include in mesh (-1 for end)
        zScale          speed of scan: distance in mm traveled per sec
        minIndex        lower bound of points to include from a scan
        maxIndex        upper bound of points to include from a scan
        timeDependent   plot scans dependent on time captured (scans will not be evenly spaced)
        cullDistance    discard all points within this distance (mm) to lidar
        color           color of points
     */
    
    void drawLinearMesh();
    
    ofParameterGroup linearParams;
    ofParameter<float> linearScale;
    ofParameter<float> linearSlide;
    ofParameter<float> linearSlideStep;
    ofParameter<bool> linearAutoSlide;
    ofParameter<float> linearAutoSlideStep;
    ofParameter<float> linearXTranslation;
    ofParameter<float> linearYTranslation;
    ofParameter<float> xRotation;
    ofParameter<float> yRotation;
    ofParameter<float> zRotation;
    ofParameter<bool> mirrorX;
    ofParameter<bool> mirrorY;
    ofParameter<bool> mirrorZ;
    
    float linearSlideLerp;
    float linearSlideLerpAmt = 0.05;
    
    
    // ---------------------------
    // ----- SPHERICAL MESH ------
    // ---------------------------
    
    ofMesh sphericalMesh;
    
    void loadSphericalData(string fileName);
    
    ofBuffer sphericalBuffer;
    unsigned long nSphericalScans;
    
    void fillSphericalMesh(float speed = 225./64., float period = 180, float startingPeriod = 0, float nPeriods = 1, int minIndex = 0, int maxIndex = 682, bool clockwise = true, int cullDistance = 265, float alignmentAngle = 0, ofColor color = ofColor(255), bool cullDuplicateScans = true);
    /*  
        speed           speed of rotating lidar (degrees / sec)
        period          degrees in one period of rotation of the lidar
        startingPeriod  period of revolution at which to start loading points
        nPeriods        number of periods to load of data (1 = single scan)
        minIndex        lower bound of points to include from a scan
        maxIndex        upper bound of points to include from a scan
        clockwise       whether lidar was rotating clockwise
        cullDistance    discard all points within this distance (mm) to lidar
        alignmentAngle  offset a single scan by this angle to align edges of hemisphere
        cullDoubleScans scans are sometimes output by the sensor twice in a row, within 30 ms of each other; this will cull doubles
     */
    
    void drawSphericalMesh(bool cameraOn = false);
    
    ofEasyCam easyCam;
    
    ofParameterGroup sphericalParams;
    ofParameter<float> sphericalScale;
    ofParameter<float> sphericalRotation;
    ofParameter<float> sphericalRotationStep;
    ofParameter<bool> sphericalAutoRotation;
    ofParameter<float> sphericalAutoRotationStep;
    ofParameter<float> sphericalXTranslation;
    ofParameter<float> sphericalYTranslation;
    ofParameter<bool> flipX;
    ofParameter<bool> flipY;
    ofParameter<bool> flipZ;
    
    float sphericalRotationLerp;
    float sphericalRotationLerpAmt = 0.05;
    
    
    // ---------------------------
    // ---------- UTILS ----------
    // ---------------------------
    
    void setKeyPressed(int key_);
    int lkey, skey;
    int getLKey();
    int getSKey();
    
};

#endif /* defined(__urg_capture_display__urgDisplay__) */
