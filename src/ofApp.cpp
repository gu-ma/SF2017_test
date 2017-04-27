#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // general
    // ofSetBackgroundAuto(false);
    varSetup();
    ofSetBackgroundColor(0);
    ofSetWindowPosition(500,500);
    // GUI
    gui.setup();
    // ft
    ft.setup("../../../../models/shape_predictor_68_face_landmarks.dat");
    ft.setDrawStyle(ofxDLib::lines);
    ft.setSmoothingRate(smoothingRate);
    // grid
    grid.init(gridWidth, gridHeight, gridRes*gridScale, gridMinSize, gridMaxSize, true);
    // video recording
    vidRecorder.init("output", "face", ".mov", "mpeg4", "100k");
    // capture
    #ifdef _USE_LIVE_VIDEO
        cam.setDeviceID(1);
        cam.setup(1920, 1080);
//        cam.setDeviceID(0);
//        cam.setup(1280, 720);
    #else
        movie.load("vids/test.mov");  // 1280x720
        movie.play();
    #endif
    
}

//--------------------------------------------------------------
void ofApp::update(){
    //
    bool newFrame = false;
    //
    #ifdef _USE_LIVE_VIDEO
        cam.update();
        newFrame = cam.isFrameNew();
    #else
        movie.update();
        newFrame = movie.isFrameNew();
    #endif
    
    if(newFrame){
        // capture
        #ifdef _USE_LIVE_VIDEO
            inputImg.setFromPixels(cam.getPixels());
        #else
            inputImg.setFromPixels(movie.getPixels());
        #endif
        inputImg.mirror(false, true);
        inputImg.crop( (inputImg.getWidth()-inputImg.getHeight())/2, 0, inputImg.getHeight(), inputImg.getHeight() );
        // resize
        scaledImg = inputImg;
        scaledImg.resize(scaledImg.getWidth()/downSize, scaledImg.getHeight()/downSize);
        // filters
        if (inputIsFiltered) {
            clahe.filter(scaledImg, scaledImg, claheClipLimit, inputIsClaheColored);
            scaledImg.update();
        }
        if (outputIsFiltered) {
            clahe.filter(inputImg, inputImg, claheClipLimit, inputIsClaheColored);
            inputImg.update();
        }
        inputPixels = inputImg.getPixels();
        //
        ft.findFaces(scaledImg.getPixels(),false);
        vector<ofxDLib::Face> faces = ft.getFaces();
        // grid
        vector<ofGrid::PixelsItem> pis;
        vector<ofGrid::TextItem> tis;
        int i = 0;
        for (auto & face : faces) {
            // grab face elements
            for (int i=0; i<20; i++) {
                vector<ofPolyline> facePolylines {ofPolyline::fromRectangle(face.rect), face.leftEye, face.rightEye, face.noseTip, face.outerMouth};
                int j = 0;
                for (auto & facePolyline : facePolylines) {
                    if (i < faceElementsCount.at(j)) {
                        // WARNING WITH MEMORY
                        // Add scale for the zoom and offset
                         pis.push_back(ofGrid::PixelsItem(getFacePart(inputPixels, facePolyline, faceElementsZoom, i*faceElementsOffset, true), ofGrid::rightEye));
                    }
                    j++;
                }
            }
            // focus and record 1 face
            if (face.age > 30 && !isFocused) {
                focusedFaceLabel = face.label;
                isFocused = true;
            }
            if (isFocused && focusedFaceLabel == face.label ){
                vidRecorder.update(getFacePart(inputPixels, face.noseTip, 1, 0, true));
                if (!isRecording) {
                    vidRecorder.start(256, 256, (int)ofGetFrameRate());
                    isRecording = true;
                }
            }
        }
        grid.updatePixels(pis);
        
        // grid txt
        vector<string> txt = { "I'M WATCHING", "THIS WORDS THIS IS THE", "STRETCH" };
        for (int i=0; i<3; i++) {
            ofGrid::TextItem ti(txt.at(i), i+1);
            tis.push_back(ti);
            ti.clear();
        }
        grid.updateText(tis);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    if (inputIsFiltered) scaledImg.draw(0, 0, 192, 192);
    else scaledImg.draw(0, 0, 192, 192);
    ofPushMatrix();
        ofScale(192/scaledImg.getWidth(), 192/scaledImg.getWidth());
        ft.draw();
    ofPopMatrix();
    //
    if (showGrid) grid.draw();
    else if (showGridElements) grid.drawGridElements();
    //
    guiDraw();
}


//--------------------------------------------------------------
ofPixels ofApp::getFacePart(ofPixels facePixels, ofPolyline partPolyline, float zoom, float offset, bool isSquare){
    ofPoint center = partPolyline.getCentroid2D();
    int w,h;
    // define width and height
    if (isSquare) {
        if ( partPolyline.getBoundingBox().width > partPolyline.getBoundingBox().height ) {
            w = partPolyline.getBoundingBox().width;
        } else {
            w = partPolyline.getBoundingBox().height;
        }
        h = w;
    } else {
        w = partPolyline.getBoundingBox().width;
        h = partPolyline.getBoundingBox().height;
    }
    // check if out of bound
    int x = center.x+offset-w/2;
    int y = center.y+offset-h/2;
    if ( x+w/2 > scaledImg.getWidth() ) x = scaledImg.getWidth()-w/2;
    else if ( x-w/2 < 0 ) x = w/2;
    if ( y+h/2 > scaledImg.getHeight() ) y = scaledImg.getHeight()-h/2;
    else if ( y-h/2 < 0 ) y = h/2;
    //
    facePixels.crop(x*downSize, y*downSize, w*downSize*1/zoom, h*downSize*1/zoom);
    return facePixels;
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'h') grid.generateGridElements();
    if (key == 'g') showGrid = !showGrid;
    if (key == 'f') showGridElements = !showGridElements;
    if (key == 'r') vidRecorder.start(256, 256, (int)ofGetFrameRate());
    if (key == 's') vidRecorder.stop();
}


//--------------------------------------------------------------
void ofApp::varSetup(){
    // capture
    downSize = 1.3;
    // ft
    smoothingRate = 1;
    enableTracking = true;
    isFocused = false;
    // filter
    claheClipLimit = 2;
    inputIsFiltered = true;
    inputIsClaheColored = false;
    outputIsFiltered = true;
    // grid
    showGrid = false;
    showGridElements = false;
    gridWidth = 6;
    gridHeight = 6;
    gridRes = 32;
    gridScale = 1;
    gridMinSize = 0;
    gridMaxSize = 8;
    gridIsSquare = true;
    // ft capture
    faceElementsCount.assign(5, 5);
    faceElementsOffset = 0;
    faceElementsZoom = .4;
}


//--------------------------------------------------------------
void ofApp::guiDraw(){
    // GUI
    gui.begin();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::SliderFloat("Down size input", &downSize, 1, 4);
        ImGui::SliderInt("claheClipLimit", &claheClipLimit, 0, 6);
        ImGui::Checkbox("isFiltered", &inputIsFiltered);
        ImGui::Checkbox("isClaheColored", &inputIsClaheColored);
        ImGui::SliderFloat("smoothingRate", &smoothingRate, 0, 6);
        if (ImGui::CollapsingHeader("Grid", false)) {
            ImGui::SliderInt("gridWidth", &gridWidth, 1, 24);
            ImGui::SliderInt("gridHeight", &gridHeight, 1, 24);
            ImGui::SliderInt("gridScale", &gridScale, 1, 12);
            ImGui::InputInt("gridRes", &gridRes, 8);
            ImGui::SliderInt("gridMaxSize", &gridMaxSize, 1, 12);
            ImGui::Checkbox("gridIsSquare", &gridIsSquare);
            ImGui::Checkbox("showGrid", &showGrid);
            if(ImGui::Button("Refresh Grid")) grid.init(gridWidth, gridHeight, gridRes*gridScale, gridMinSize, gridMaxSize, gridIsSquare);
        }
        if (ImGui::CollapsingHeader("Elements", false)) {
            ImGui::SliderInt("face Count", &faceElementsCount[0], 0, 20);
            ImGui::SliderInt("Left Eye Count", &faceElementsCount[1], 0, 20);
            ImGui::SliderInt("Right Eye Count", &faceElementsCount[2], 0, 20);
            ImGui::SliderInt("Nose Count", &faceElementsCount[3], 0, 20);
            ImGui::SliderInt("Mouth Count", &faceElementsCount[4], 0, 20);
            ImGui::SliderFloat("Offset", &faceElementsOffset, 0, 10);
            ImGui::SliderFloat("Zoom", &faceElementsZoom, 0.2, 6);
        }
    gui.end();
}


//--------------------------------------------------------------
void ofApp::exit(){
    vidRecorder.close();
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
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
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
