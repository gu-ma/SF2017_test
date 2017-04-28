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
    grid.init(gridWidth, gridHeight, gridRes, gridMinSize, gridMaxSize, true);
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
        inputImg.mirror(false, true);
        inputImg.crop((inputImg.getWidth()-inputImg.getHeight())/2, 0, inputImg.getHeight(), inputImg.getHeight());
        // resize
        scaledImg = inputImg;
        scaledImg.resize(scaledImg.getWidth()/downSize, scaledImg.getHeight()/downSize);
        scaledImg.update();
        // filters
        if (inputIsFiltered) {
            clahe.filter(scaledImg, scaledImgFiltered, claheClipLimit, inputIsClaheColored);
            scaledImgFiltered.update();
            ft.findFaces(scaledImgFiltered.getPixels(), false);
        } else {
            ft.findFaces(scaledImg.getPixels(),false);
        }
        if (outputIsFiltered) {
            clahe.filter(inputImg, inputImgFiltered, claheClipLimit, inputIsClaheColored);
            inputImgFiltered.update();
            inputPixels = inputImgFiltered.getPixels();
        } else {
            inputPixels = inputImg.getPixels();
        }
        
        //
        if ( ft.size() == 0 ) {
            // *********
            // No faces are detected
            if (facesFound) {
                // start iddle timer
                timer01.reset();
                timer01.startTimer();
                // reset variables
                facesFound = false;
                isFocused = false;
                isRecording = false;
                // stop vid player
                vidRecorder.stop();
            }
            if (timer01.isTimerFinished()) {
                // *********
                // Idle mode
                // No faces are detected for more than XX seconds
                showCapture = false;
            }
        } else {
            // *********
            // Faces are detected
            facesFound = true;
            showCapture = true;
            // get faces
            faces = ft.getFaces();
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
                            pis.push_back(ofGrid::PixelsItem(getFacePart(inputPixels, facePolyline, downSize, faceElementsZoom, i*faceElementsOffset, true), ofGrid::rightEye));
                        }
                        j++;
                    }
                }

                // *********
                // One face is present more than 4 seconds
                // Save face label and set isFocused to true
                if (face.age > focusTime && !isFocused) {
                    focusedFaceLabel = face.label;
                    isFocused = true;
                }
                // if the label of the face is the one focused on
                if (isFocused && face.label == focusedFaceLabel){
                    focusedFace = face;
                    // record frames
//                    ofRectangle r;
//                    r.setFromCenter(face.noseTip.getCentroid2D(), 100, 100);
                    ofPixels faceCropped = getFacePart(inputPixels, ofPolyline::fromRectangle(face.rect), downSize, .5, 0, true);
                    vidRecorder.update(faceCropped);
                    // start the recording
                    if (!isRecording) {
                        vidRecorder.start("output/face", ofToString(face.label),  256, 256, (int)ofGetFrameRate());
                        isRecording = true;
                    }
                    //
                    // start the 2nd timer
                    timer02.reset();
                    timer02.startTimer();
                    // zoom in
                    

                    
                }
            }
            grid.updatePixels(pis);
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
        ofScale(gridScale, gridScale);
        if (showCapture){
            if (isFocused) {
                int focusSize = 400;
                int x = focusedFace.rect.getCenter().x - focusSize/2;
                int y = focusedFace.rect.getCenter().y - focusSize/2;
                x = ofClamp(x, 0, inputPixels.getWidth()*downSize);
                y = ofClamp(y, 0, inputPixels.getHeight()*downSize);
                if (inputIsFiltered) inputImgFiltered.drawSubsection(0, 0, 192, 192, x*downSize, y*downSize, focusSize*downSize, focusSize*downSize);
                else inputImg.drawSubsection(0, 0, 192, 192, x*downSize, y*downSize, focusSize*downSize, focusSize*downSize);
            } else {
                if (inputIsFiltered) inputImgFiltered.draw(0, 0, 192, 192);
                else inputImg.draw(0, 0, 192, 192);
                // draw facetracker
                ofPushMatrix();
                ofPushStyle();
                ofScale(192/scaledImg.getWidth(), 192/scaledImg.getWidth());
                ft.draw();
                if (isFocused) {
                    ofNoFill();
                    ofSetLineWidth(2);
                    for (auto & face : faces) {
                        if (face.label == focusedFaceLabel) {
                            ofDrawRectangle(face.rect);
                        }
                    }
                }
                ofPopStyle();
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
//    // convert x and y
//    int x = center.x*downScale+offset;
//    int y = center.y*downScale+offset;
//    // define width and height
//    int w,h;
//    if (isSquare) {
//        if ( partPolyline.getBoundingBox().width > partPolyline.getBoundingBox().height ) {
//            w = partPolyline.getBoundingBox().width*downScale*1/zoom;
//        } else {
//            w = partPolyline.getBoundingBox().height*downScale*1/zoom;
//        }
//        h = w;
//    } else {
//        w = partPolyline.getBoundingBox().width*downScale*1/zoom;
//        h = partPolyline.getBoundingBox().height*downScale*1/zoom;
//    }
//    // check if out of bound
////     x = ofClamp(x, )
//    if (x+w/2 > sourcePixels.getWidth()*downScale) x = sourcePixels.getWidth()*downScale-w/2;
//    else if ( x-w/2 < 0 ) x = w/2;
//    if (y+h/2 > sourcePixels.getHeight()*downScale) y = sourcePixels.getHeight()*downScale-h/2;
//    else if ( y-h/2 < 0 ) y = h/2;
//    else {
//        x = x-w/2;
//        y = y-h/2;
//    }
//    //
//    sourcePixels.crop(x, y, w, h);
//    return sourcePixels;
    // def x and y
    int x = center.x+offset;
    int y = center.y+offset;
    // define width and height
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
//    x = ofClamp(x, w/2, sourcePixels.getWidth()*downScale-w/2);
//    y = ofClamp(y, h/2, sourcePixels.getHeight()*downScale-h/2);
//    if (x+w/2 > sourcePixels.getWidth()) x = sourcePixels.getWidth()-w/2;
//    else if ( x-w/2 < 0 ) x = w/2;
//    if (y+h/2 > sourcePixels.getHeight()) y = sourcePixels.getHeight()-h/2;
//    else if ( y-h/2 < 0 ) y = h/2;
//    else {
//        x = x-w/2;
//        y = y-h/2;
//    }
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
void ofApp::varSetup(){
    // capture
    downSize = 1.3;
    showCapture = true;
    //
    timeOut01 = 2000;
    timeOut02 = 3000;
    //
    timer01.setup(timeOut01, false);
    timer02.setup(timeOut02, false);
    // ft
    focusTime = 40;
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
        ImGui::Checkbox("inputIsFiltered", &inputIsFiltered);
        ImGui::Checkbox("outputIsFiltered", &outputIsFiltered);
        ImGui::Checkbox("isClaheColored", &inputIsClaheColored);
        if (ImGui::SliderFloat("smoothingRate", &smoothingRate, 0, 6)) ft.setSmoothingRate(smoothingRate);
        if (ImGui::CollapsingHeader("Grid", false)) {
            ImGui::SliderInt("gridWidth", &gridWidth, 1, 24);
            ImGui::SliderInt("gridHeight", &gridHeight, 1, 24);
            ImGui::SliderInt("gridScale", &gridScale, 1, 12);
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
