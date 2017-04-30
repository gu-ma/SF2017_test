#pragma once

#include "ofMain.h"
#include "FaceTracker.h"
#include "ofxVideoRecorder.h"
#include "ofxImGui.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxTimer.h"
// local files
#include "Clahe.h"
#include "ofGrid.h"
#include "ofVidRec.h" // encapsulate ofxVideoRecorder.h for convenience

#define _USE_LIVE_VIDEO

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
    void randomizeSettings();
    int sceneScale;
    bool isIdle, showIdle;
    // Video Player
    void loadVideos(), drawVideos(), stopVideos(), updateVideos() ;
    int currentVideo, dirSize, countVideos;
    ofDirectory dir;
    vector<ofVideoPlayer> videosArray;
    bool showVideos, reloadVideos;
    // timers
    ofxTimer timer01, timer02, timer03;
    int timeOut01, timeOut02, timeOut03;
    // capture
    ofVideoGrabber cam;
    ofVideoPlayer movie;
    ofImage inputImg, inputImgFiltered, scaledImg, scaledImgFiltered;
    ofPixels inputPixels;
    float downSize;
    bool showCapture;
    // filter
    int claheClipLimit;
    Clahe clahe;
    bool inputIsFiltered, inputIsClaheColored, imgIsFiltered;
    // ft
    ofxDLib::FaceTracker ft;
    vector<ofxDLib::Face> faces;
    ofxDLib::Face focusedFace;
    ofPixels getFacePart(ofPixels sourcePixels, ofPolyline partPolyline, float downScale, float zoom, float offset, bool isSquare);
    float smoothingRate;
    bool enableTracking, isFocused, facesFound;
    int focusedFaceLabel, focusTime, faceTotalFrame;
    float faceAvgWidth, faceAvgHeight, faceTotalWidth, faceTotalHeight;
    // Always the same order: face, leftEye, rightEye, mouth, nose
    vector<int> faceElementsCount;
    float faceElementsOffset, faceElementsZoom;
    // grid
    bool showGrid, showGridElements;
    int gridWidth, gridHeight, gridRes, gridMinSize, gridMaxSize;
    bool gridIsSquare;
    ofGrid grid;
    // video recording
    ofVidRec vidRecorder;
    bool isRecording;
    string faceVideoPath;
    // GUI
    ofxImGui::Gui gui;
    void guiDraw();
    
};
