#pragma once

#include "ofMain.h"
#include "FaceTracker.h"
#include "ofxVideoRecorder.h"
#include "ofxImGui.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
// local files
#include "Clahe.h"
#include "ofGrid.h"
#include "ofVidRec.h" // encapsulate ofxVideoRecorder.h for convenience

//#define _USE_LIVE_VIDEO

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void exit();
    
    // General
    void varSetup();
    //
    
    // capture
    ofVideoGrabber cam;
    ofVideoPlayer movie;
    ofImage colorImg, filteredImg, resizedImg;
    float downSize;
    // filter
    int claheClipLimit;
    Clahe clahe;
    bool isFiltered, isClaheColored;
    // ft
    ofxDLib::FaceTracker ft;
    ofPixels getFacePart(ofPixels pixels, ofPolyline partPolyline, float zoom, float offset, bool isSquare);
    // Always the same order: face, leftEye, rightEye, mouth, nose
    vector<int> faceElementsCount;
    float faceElementsOffset, faceElementsZoom;
    // grid
    bool showGrid;
    int gridWidth, gridHeight, gridRes, gridScale, gridMinSize, gridMaxSize;
    bool gridIsSquare;
    ofGrid grid;
    // video recording
    ofVidRec vidRecorder;
    // GUI
    ofxImGui::Gui gui;
    void guiDraw();
    
};
