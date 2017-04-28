#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // general
    // ofSetBackgroundAuto(false);
    varSetup();
    ofSetBackgroundColor(0);
    ofSetWindowPosition(100,100);
    // GUI
    gui.setup();
    // ft
    ft.setup("../../../../models/shape_predictor_68_face_landmarks.dat");
    ft.setDrawStyle(ofxDLib::lines);
    ft.setSmoothingRate(smoothingRate);
    // grid
    grid.init(gridWidth, gridHeight, gridRes, gridMinSize, gridMaxSize, gridIsSquare);
    // video recording
    vidRecorder.init(".mov", "mpeg4", "100k");
    // capture
    #ifdef _USE_LIVE_VIDEO
        cam.setDeviceID(0);
        cam.setup(1920, 1080);
        cam.setPixelFormat(OF_PIXELS_RGB);
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
        // mirror and crop the input
        inputImg.mirror(false, true);
        inputImg.crop((inputImg.getWidth()-inputImg.getHeight())/2, 0, inputImg.getHeight(), inputImg.getHeight());
        // resize the input image to feed to the tracker to improve the frame rate
        scaledImg = inputImg;
        scaledImg.resize(scaledImg.getWidth()/downSize, scaledImg.getHeight()/downSize);
        scaledImg.update();
        // Filter input image
        if (inputIsFiltered) {
            clahe.filter(scaledImg, scaledImgFiltered, claheClipLimit, inputIsClaheColored);
            scaledImgFiltered.update();
            // ft look for faces
            ft.findFaces(scaledImgFiltered.getPixels(), false);
        } else {
            // ft look for faces
            ft.findFaces(scaledImg.getPixels(),false);
        }
        // Filter output image
        if (outputIsFiltered) {
            clahe.filter(inputImg, inputImgFiltered, claheClipLimit, inputIsClaheColored);
            inputImgFiltered.update();
            // assign the inputPixels
            inputPixels = inputImgFiltered.getPixels();
        } else {
            // assign the inputPixels
            inputPixels = inputImg.getPixels();
        }
        
        // start the "logic" ;)
        if ( ft.size() == 0 ) {
            // *********
            // Faces were found but are not detected anymore
            if (facesFound) {
                // start iddle timer
                timer01.reset();
                timer01.startTimer();
                // reset bool
                facesFound = false;
                isFocused = false;
                isRecording = false;
                showGrid = false;
                // stop vid player
                vidRecorder.stop();
                // reset avg face w / h
                faceAvgWidth = 0;
                faceAvgHeight = 0;
                faceTotalFrame = 0;
                faceTotalWidth = 0;
                faceTotalHeight = 0;
            }
            if (timer01.isTimerFinished()) {
                // *********
                // Idle mode
                // No faces are detected for more than XX seconds
                showCapture = false;
                focusTime = 40 + (timeOut02/100);
            }
        } else {
            // *********
            // Faces are detected
            if (!facesFound) {
                facesFound = true;
                // start 2nd timer
                timer02.reset();
                timer02.startTimer();
            }
            if (timer02.isTimerFinished()) showCapture = true;
            //
            if (showCapture) {
                // get faces
                faces = ft.getFaces();
                vector<ofGrid::PixelsItem> pis;
                vector<ofGrid::TextItem> tis;
                int i = 0;
                for (auto & face : faces) {
                    
                    // *********
                    // One face is present more than XX seconds
                    // Save face label and set isFocused to true
                    if (face.age > focusTime && !isFocused) {
                        focusedFaceLabel = face.label;
                        isFocused = true;
                        // start the timer
                        timer03.reset();
                        timer03.startTimer();
                        focusTime = 40;
                    }
                    
                    // if the face is the one focused on
                    if (isFocused && face.label == focusedFaceLabel){
                        focusedFace = face;
                        // create a rectangle with the average width and height of the face
                        faceTotalFrame ++;
                        faceTotalWidth += face.rect.getWidth();
                        faceTotalHeight += face.rect.getHeight();
                        faceAvgWidth = faceTotalWidth / faceTotalFrame;
                        faceAvgHeight = faceTotalHeight / faceTotalFrame;
                        ofRectangle r;
                        //                    ofLog(OF_LOG_NOTICE, "faceAvgWidth " + ofToString(faceAvgWidth));
                        r.setFromCenter(face.rect.getCenter(), faceAvgWidth, faceAvgHeight);
                        // extract the face and record it
                        ofPixels faceCropped = getFacePart(inputPixels, ofPolyline::fromRectangle(r), downSize, .5, 0, true);
                        vidRecorder.update(faceCropped);
                        // if not yet recording start the recording
                        if (!isRecording) {
                            vidRecorder.start("output/face", ofToString(face.label),  256, 256, (int)ofGetFrameRate());
                            isRecording = true;
                        }
                        // after a certain time, show the grid
                        if (timer03.isTimerFinished()) {
                            if (!showGrid) {
                                randomizeSettings();
                                grid.init(gridWidth, gridHeight, gridRes, gridMinSize, gridMaxSize, gridIsSquare);
                                showGrid = true;
                            }
                        }
                    }
                    
                    // select face elements for the grid
                    for (int i=0; i<20; i++) {
                        vector<ofPolyline> facePolylines {ofPolyline::fromRectangle(face.rect), face.leftEye, face.rightEye, face.noseTip, face.outerMouth};
                        int j = 0;
                        for (auto & facePolyline : facePolylines) {
                            if (i < faceElementsCount.at(j)) {
                                // WARNING WITH MEMORY
                                pis.push_back(ofGrid::PixelsItem(getFacePart(inputPixels, facePolyline, downSize, faceElementsZoom, i*faceElementsOffset, true), ofGrid::rightEye));
                            }
                            j++;
                        }
                    }
                }
                grid.updatePixels(pis);
            }
        }
        
        // grid txt
//        vector<string> txt = { "I'M WATCHING", "THIS WORDS THIS IS THE", "STRETCH" };
//        for (int i=0; i<3; i++) {
//            ofGrid::TextItem ti(txt.at(i), i+1);
//            tis.push_back(ti);
//            ti.clear();
//        }
//        grid.updateText(tis);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    // Draw tracked area
    ofPushMatrix();
        ofScale(sceneScale, sceneScale);
        if (showCapture){
            if (isFocused) {
                int focusSize = faceAvgWidth*2;
                int x = focusedFace.rect.getCenter().x - focusSize/2;
                int y = focusedFace.rect.getCenter().y - focusSize/2;
                x = ofClamp(x, 0, inputPixels.getWidth()*downSize);
                y = ofClamp(y, 0, inputPixels.getHeight()*downSize);
                if (inputIsFiltered) inputImgFiltered.drawSubsection(0, 0, 192, 192, x*downSize, y*downSize, focusSize*downSize, focusSize*downSize);
                else inputImg.drawSubsection(0, 0, 192, 192, x*downSize, y*downSize, focusSize*downSize, focusSize*downSize);
                ofPushStyle();
                    ofSetColor(28, 249, 255);
                    int w = 2 * sceneScale;
                    ofDrawRectangle(0, 0, 192, w);
                    ofDrawRectangle(192-w, 0, w, 192);
                    ofDrawRectangle(0, 192-w, 192, w);
                    ofDrawRectangle(0, 0, w, 192);
                ofPopStyle();
            } else {
                if (inputIsFiltered) inputImgFiltered.draw(0, 0, 192, 192);
                else inputImg.draw(0, 0, 192, 192);
                // draw facetracker
                ofPushMatrix();
//                ofPushStyle();
                ofScale(192/scaledImg.getWidth(), 192/scaledImg.getWidth());
                ft.draw();
//                if (isFocused) {
//                    ofNoFill();
//                    ofSetLineWidth(2);
//                    for (auto & face : faces) {
//                        if (face.label == focusedFaceLabel) {
//                            ofDrawRectangle(face.rect);
//                        }
//                    }
//                }
//                ofPopStyle();
                ofPopMatrix();
            }
        }
        // draw grid
        if (showGrid) grid.draw();
        else if (showGridElements) grid.drawGridElements();
    ofPopMatrix();
    //
    guiDraw();
}


//--------------------------------------------------------------
ofPixels ofApp::getFacePart(ofPixels sourcePixels, ofPolyline partPolyline, float downScale, float zoom, float offset, bool isSquare){
    ofPoint center = partPolyline.getCentroid2D();
    // def x and y
    int x = center.x+offset;
    int y = center.y+offset;
    // def width and height
    int w,h;
    if (isSquare) {
        if ( partPolyline.getBoundingBox().width > partPolyline.getBoundingBox().height ) {
            w = partPolyline.getBoundingBox().width*1/zoom;
        } else {
            w = partPolyline.getBoundingBox().height*1/zoom;
        }
        h = w;
    } else {
        w = partPolyline.getBoundingBox().width*1/zoom;
        h = partPolyline.getBoundingBox().height*1/zoom;
    }
    // check if out of bound
    x = x-w/2;
    y = y-h/2;
    x = ofClamp(x, 1, sourcePixels.getWidth()*downScale-1);
    y = ofClamp(y, 1, sourcePixels.getHeight()*downScale-1);
    //
    sourcePixels.crop(x*downScale, y*downScale, w*downScale, h*downScale);
    return sourcePixels;
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'h') grid.generateGridElements();
    if (key == 'g') showGrid = !showGrid;
    if (key == 'f') showGridElements = !showGridElements;
//    if (key == 'r') vidRecorder.start(256, 256, (int)ofGetFrameRate(),0);
    if (key == 's') vidRecorder.stop();
}


//--------------------------------------------------------------
void ofApp::randomizeSettings(){
    // ft
    for (auto & f : faceElementsCount) {
        f = ofRandom(0,20);
    }
    faceElementsOffset = ofRandom(0, 1);
    faceElementsZoom = ofRandom(0.5, 1);
    // grid
    if (ofRandom(1)>.5) {
        gridWidth = 6;
        gridHeight = 6;
        gridRes = 32;
        gridMaxSize = ofRandom(6);
    } else {
        gridWidth = 12;
        gridHeight = 12;
        gridRes = 16;
        gridMaxSize = ofRandom(12);
    }
    gridIsSquare = (ofRandom(1)>.5) ? true : false;
}

//--------------------------------------------------------------
void ofApp::varSetup(){
    // capture
    downSize = 1.3;
    showCapture = true;
    //
    timeOut01 = 5000; // time before iddle
    timeOut02 = 3000; // time before showCapture
    timeOut03 = 4000; // time before grid
    //
    timer01.setup(timeOut01, false);
    timer02.setup(timeOut02, false);
    timer03.setup(timeOut03, false);
    // ft
    focusTime = 40; // time before focusing + recording
    smoothingRate = 1;
    enableTracking = true;
    isFocused = false;
    faceAvgWidth = 0;
    faceAvgHeight = 0;
    faceTotalFrame = 0;
    faceTotalWidth = 0;
    faceTotalHeight = 0;
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
    sceneScale = 1;
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
        ImGui::SliderInt("sceneScale", &sceneScale, 1, 12);
        ImGui::SliderInt("claheClipLimit", &claheClipLimit, 0, 6);
        ImGui::Checkbox("inputIsFiltered", &inputIsFiltered);
        ImGui::Checkbox("outputIsFiltered", &outputIsFiltered);
        ImGui::Checkbox("isClaheColored", &inputIsClaheColored);
        if (ImGui::SliderFloat("smoothingRate", &smoothingRate, 0, 6)) ft.setSmoothingRate(smoothingRate);
        if (ImGui::CollapsingHeader("Grid", false)) {
            ImGui::SliderInt("gridWidth", &gridWidth, 1, 24);
            ImGui::SliderInt("gridHeight", &gridHeight, 1, 24);
            ImGui::InputInt("gridRes", &gridRes, 8);
            ImGui::SliderInt("gridMaxSize", &gridMaxSize, 1, 12);
            ImGui::Checkbox("gridIsSquare", &gridIsSquare);
            ImGui::Checkbox("showGrid", &showGrid);
            if(ImGui::Button("Refresh Grid")) grid.init(gridWidth, gridHeight, gridRes, gridMinSize, gridMaxSize, gridIsSquare);
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
