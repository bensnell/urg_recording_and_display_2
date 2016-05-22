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
    void fillLinearMesh(int startScan = 0, int endScan = -1, int zScale = 30, int minIndex = 0, int maxIndex = 682, bool timeDependent = false, int cullDistance = 265, ofColor color = ofColor(255));
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
    ofParameter<float> xTranslation;
    ofParameter<float> yTranslation;
    ofParameter<float> xRotation;
    ofParameter<float> yRotation;
    ofParameter<float> zRotation;
    ofParameter<bool> mirrorX;
    ofParameter<bool> mirrorY;
    
    float linearSlideLerp;
    float linearSlideLerpAmt = 0.05;
    
    
    // ---------------------------
    // ----- SPHERICAL MESH ------
    // ---------------------------
    
    ofMesh sphericalMesh;
    
    void loadSphericalData(string fileName);
    
    void fillSphericalMesh(float speed = 225./64., float startingPeriod = 0, float nPeriods = 1, int minIndex = 0, int maxIndex = 682, bool timeDependent = false, bool clockwise = true, int cullDistance = 265, float alignmentAngle = 0, ofColor color = ofColor(255));
    /*  
        speed           speed of rotating lidar (degrees / sec)
        startingPeriod  period of revolution at which to start loading points
        nPeriods        number of periods to load of data (1 = single scan)
        minIndex        lower bound of points to include from a scan
        maxIndex        upper bound of points to include from a scan
        timeDependent   plot scans dependent on time captured (scans will not be evenly spaced)
        clockwise       whether lidar was rotating clockwise
        cullDistance    discard all points within this distance (mm) to lidar
        alignmentAngle  offset a single scan by this angle to align edges of hemisphere
     */
    
    void drawSphericalMesh();
    
    //void drawPointMeshSpherical(float scale, float slide);
    
    
    
    // ---------------------------
    // -------- SAVE MESH --------
    // ---------------------------

    // ply format
    
    
    
    ofCamera camera;
    
    ofEasyCam easyCam;
    
    void setKeyPressed(int key_);
    int key;
    int getKeyPressed();
    
};

#endif /* defined(__urg_capture_display__urgDisplay__) */
