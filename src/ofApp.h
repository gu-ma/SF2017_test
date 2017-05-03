#pragma once

#include "ofMain.h"
#include "FaceTracker.h"
#include "ofxVideoRecorder.h"
#include "ofxImGui.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxTimer.h"
#include "ofxAbletonLive.h"
#include "ofxeasing.h"
#include "ofxBlackMagic.h"
// local files
#include "Clahe.h"
#include "ofGrid.h"
#include "ofVidRec.h" // encapsulate ofxVideoRecorder.h for convenience

#define _USE_LIVE_VIDEO
//#define _USE_BLACKMAGIC

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
    int dirSize, countVideos;
    ofDirectory dir;
    vector<ofVideoPlayer> videosVector;
    bool playVideos;
    
    // timers
    ofxTimer timer01, timer02, timer03, timer04;
    int timeOut01, timeOut02, timeOut03, timeOut04;
    
    // BlackMagic
    ofxBlackMagic blackCam;

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
    bool inputIsFiltered, inputIsColored, imgIsFiltered, imgIsColored;
    
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
    vector<int> faceElementsQty;
    vector<float> faceElementsZoom;
    vector<float> faceElementsOffset;
//    float faceElementsOffset, faceElementsZoom;
    
    // grid
    bool showGrid, showGridElements;
    int gridWidth, gridHeight, gridRes, gridMinSize, gridMaxSize;
    float initTimeGrid;
    bool gridIsSquare;
    ofGrid grid;
    
    // video recording
    ofVidRec vidRecorder;
    bool isRecording;
    string faceVideoPath;
    
    // Ableton live
    ofxAbletonLive live;
    void refreshLive(), initLive();
    vector<float> volumes, startVolumes, endVolumes;
    vector<float> initTimesVolumes;
    bool resetLive;
    
    // GUI
    ofxImGui::Gui gui;
    void guiDraw();
    
    //Text
    vector<ofTrueTypeFont> textDisplay;
    bool showText;
    string wrapString(string text, int width, ofTrueTypeFont textField);
};
