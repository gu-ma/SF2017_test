#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    // general
    // ofSetBackgroundAuto(false);
    ofSetVerticalSync(false);  
    varSetup();
    ofSetBackgroundColor(0);
    ofSetWindowPosition(500, 500);
    ofSetWindowShape(800, 576);
    // GUI
    gui.setup();
    // ft
    ft.setup("../../../../models/shape_predictor_68_face_landmarks.dat");
    ft.setDrawStyle(ofxDLib::lines);
    ft.setSmoothingRate(smoothingRate);
    // grid
    grid.init(gridWidth, gridHeight, gridRes, gridMinSize, gridMaxSize, gridIsSquare);
    // video recording
    vidRecorder.init(".mov", "mjpeg", "300k");
    // capture
    #ifdef _USE_LIVE_VIDEO
        cam.setDeviceID(0);
        cam.setup(1920, 1080);
    #else
        movie.load("vids/test.mov");  // 1280x720
        movie.play();
    #endif
    // Live
    live.setup();
    initLive();
}


//--------------------------------------------------------------
void ofApp::update(){
    //
    live.update();
    refreshLive();
    //
    bool newFrame = false;
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
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
        if (imgIsFiltered) {
            clahe.filter(inputImg, inputImgFiltered, claheClipLimit, inputIsClaheColored);
            inputImgFiltered.update();
            // assign the inputPixels
            inputPixels = inputImgFiltered.getPixels();
        } else {
            // assign the inputPixels
            inputImg.update();
            inputPixels = inputImg.getPixels();
        }
        
        // start the "logic" ;)
        // *********
        // Faces aren't found
        if ( ft.size() == 0 ) {
            // Faces were found before but are not detected anymore
            if (facesFound) {
                // start iddle timer
                timer01.reset();
                timer01.startTimer();
                // reset bool
                facesFound = false, isRecording = false, isFocused = false, playVideos = true, showGrid = false;
                // stop vid recorder
                vidRecorder.stop();
                // reset avg face w / h
                faceAvgWidth = 0, faceAvgHeight = 0, faceTotalFrame = 0, faceTotalWidth = 0, faceTotalHeight = 0;
                //
                focusTime = 20;
            }
            if (timer01.isTimerFinished()) {
                // *********
                // Idle mode
                // No faces are detected for more than XX seconds
                if (!isIdle) {
                    isIdle = true;
                    showCapture = false;
                    focusTime = 20 + (timeOut02/100);
                    // change the volume of track2
                    initTimes[0] = ofGetElapsedTimef(), startVolumes[0] = .2, endVolumes[0] = .1;
                    initTimes[1] = ofGetElapsedTimef(), startVolumes[1] = .6, endVolumes[1] = .1;
                    initTimes[2] = ofGetElapsedTimef(), startVolumes[2] = .4, endVolumes[2] = .2;
                    initTimes[3] = ofGetElapsedTimef(), startVolumes[3] = .4, endVolumes[3] = .2;
                    initTimes[4] = ofGetElapsedTimef(), startVolumes[4] = .6, endVolumes[4] = .2;
                }
                if (playVideos) {
                    loadVideos();
                    playVideos = false;
                    //
                }
            }
        } else {
            // *********
            // Faces are detected
            // It comes from idle mode
            if (isIdle && !facesFound) {
                // start 2nd timer
                timer02.reset();
                timer02.startTimer();
                facesFound = true;
            } else if (!facesFound) {
                facesFound = true;
            }
            // When the 2nd timer is finished show the capture
            if (timer02.isTimerFinished()) {
                timer02.reset();
                showCapture = true;
            }
            //
            if (showCapture) {
                // Stop the idle mode if necessary
                if (isIdle) {
                    stopVideos();
                    isIdle = false;
                    // Change the volume of track 1
                    initTimes[0] = ofGetElapsedTimef(), startVolumes[0] = .1, endVolumes[0] = .2;
                    initTimes[1] = ofGetElapsedTimef(), startVolumes[1] = .1, endVolumes[1] = .6;
                    initTimes[2] = ofGetElapsedTimef(), startVolumes[2] = .2, endVolumes[2] = .4;
                    initTimes[3] = ofGetElapsedTimef(), startVolumes[3] = .2, endVolumes[3] = .4;
                    initTimes[4] = ofGetElapsedTimef(), startVolumes[4] = .2, endVolumes[4] = .6;
                }
//                // Hide the grid if it's visible
//                if (showGrid) {
//                    showGrid = false;
//                }
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
                        focusTime = 20;
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
                        r.setFromCenter(face.rect.getCenter(), faceAvgWidth, faceAvgHeight);
                        // extract the face and record it
                        ofPixels faceCropped = getFacePart(inputPixels, ofPolyline::fromRectangle(r), downSize, .5, 0, true);
                        vidRecorder.update(faceCropped);
                        // if not yet recording start the recording
                        if (!isRecording) {
                            vidRecorder.start(faceVideoPath, ofToString(face.label),  256, 256, (int)ofGetFrameRate());
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
                            if (i < faceElementsQty.at(j)) {
                                // WARNING WITH MEMORY
                                pis.push_back(ofGrid::PixelsItem(getFacePart(inputPixels, facePolyline, downSize, faceElementsZoom.at(j), i*faceElementsOffset.at(j), true), ofGrid::rightEye));
                            }
                            j++;
                        }
                    }
                    
//                    // TEST
//                    // grid txt
//                    vector<string> txt = { "I'M WATCHING", "THIS WORDS THIS IS THE", "STRETCH" };
//                    for (int i=0; i<3; i++) {
//                        ofGrid::TextItem ti(txt.at(i), i+1);
//                        tis.push_back(ti);
//                        ti.clear();
//                    }
//                    grid.updateText(tis);
                    
                }
                grid.updatePixels(pis);
            }
        }
    }
}


//--------------------------------------------------------------
void ofApp::draw(){
    
    // Draw tracked area
    ofPushMatrix();
        ofScale(sceneScale, sceneScale);
    
        if (showCapture){
            //
            if (isFocused) {
                int focusSize = faceAvgWidth*2;
                int x = focusedFace.rect.getCenter().x - focusSize/2;
                int y = focusedFace.rect.getCenter().y - focusSize/2;
                x = ofClamp(x, 0, inputPixels.getWidth()*downSize);
                y = ofClamp(y, 0, inputPixels.getHeight()*downSize);
                if (inputIsFiltered) inputImgFiltered.drawSubsection(0, 0, 192, 192, x*downSize, y*downSize, focusSize*downSize, focusSize*downSize);
                else inputImg.drawSubsection(0, 0, 192, 192, x*downSize, y*downSize, focusSize*downSize, focusSize*downSize);
                ofPushStyle();
                    ofSetColor(ofColor::red);
                    int w = 2 * sceneScale;
                    ofDrawRectangle(0, 0, 192, w);
                    ofDrawRectangle(192-w, 0, w, 192);
                    ofDrawRectangle(0, 192-w, 192, w);
                    ofDrawRectangle(0, 0, w, 192);
                ofPopStyle();
            } else{
                // draw inputImg
                if (inputIsFiltered) inputImgFiltered.draw(0, 0, 192, 192);
                else inputImg.draw(0, 0, 192, 192);
                // draw facetracker
                ofPushMatrix();
                ofScale(192/scaledImg.getWidth(), 192/scaledImg.getWidth());
                ft.draw();
                ofPopMatrix();
            }
        }
        // draw grid
        if (showGrid) grid.draw();
        else if (showGridElements) grid.drawGridElements();
//        // TEST
//        if (!grid.textItems.empty()) {
//            grid.textItems.at(2).draw(0, 0, 128, 128, ofColor(255, 0, 0));
//        }
    
        //
        if (isIdle) drawVideos();
        // draw Smiley
        ofPushStyle();
        ofPushMatrix();
            // BAD
            ofTranslate(161,174);
            ofSetColor(ofColor::red);
            ofDrawCircle(6, 5, 1);
            ofDrawCircle(12, 5, 1);
            ofDrawRectangle(5, 10, 8, 1);
            ofDrawBitmapString(ofToString(ft.size()), 18, 12);
        ofPopStyle();
        ofPopMatrix();
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
void ofApp::loadVideos() {
    dir.allowExt("mov");
    dir.listDir(faceVideoPath);
    dir.sort();
    if(dir.size()>countVideos){
        videosVector.assign(countVideos, ofVideoPlayer());
    }
    // iterate through the files and load them into the vector
    int j = 0;
    // reverse browsing the dir
    for(int i=(int)dir.size()-1; i>=0 && j<countVideos; i--){
        if ( dir.getFile(i).getSize() > 100000 ) {
            videosVector[j].load(dir.getPath(i));
            videosVector[j].setLoopState(OF_LOOP_PALINDROME);
//            videosVector[j].setSpeed(30);
            videosVector[j].play();
//            ofLog(OF_LOG_NOTICE, ofToString(dir.getPath(i)));
            j++;
        }
    }
    currentVideo = 0;
}


//--------------------------------------------------------------
void ofApp::drawVideos() {
    if (videosVector.size()) {
        for(int i = 0; i < videosVector.size(); i++){
            videosVector[i].update();
            videosVector[i].draw((i%2)*96,(i/2)*96, 96, 96);
        }
    }
}


//--------------------------------------------------------------
void ofApp::stopVideos() {
    if (videosVector.size()) {
        for(int i = 0; i < videosVector.size(); i++){
            videosVector[i].stop();
            videosVector[i].closeMovie();
        }
    }
}


//--------------------------------------------------------------
void ofApp::randomizeSettings(){
    // ft
    for (auto & f : faceElementsQty) f = ofRandom(0,20);
    for (auto & f : faceElementsOffset) f = ofRandom(0, 1);
    for (int i=0; i < faceElementsZoom.size() ; i++) {
        faceElementsZoom.at(i) = (i>0) ? ofRandom(.2, .5) : ofRandom(.5, 1);
        cout << faceElementsZoom.at(i) << endl;
    }
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
void ofApp::keyPressed(int key){
    if (key == 'h') grid.generateGridElements();
    if (key == 'g') showGrid = !showGrid;
    if (key == 'f') showGridElements = !showGridElements;
//    if (key == 'r') vidRecorder.start(256, 256, (int)ofGetFrameRate(),0);
    if (key == 's') vidRecorder.stop();
    //
    if (key == '0') initLive();
    if (key == 'l') live.printAll();
    if (key == '.') {
        float v = live.getVolume();
        live.setVolume( ofClamp(v + 0.1, 0, 1) );
    }
    if (key == ',') {
        float v = live.getVolume();
        live.setVolume( ofClamp(v - 0.1, 0, 1) );
    }
}


//--------------------------------------------------------------
void ofApp::varSetup(){
    // general
    isIdle = true;
    sceneScale = 2;
    // capture
    downSize = 1.5;
    showCapture = true, facesFound = false;
    // video recording
    faceVideoPath = "output/face";
    playVideos = true;
    countVideos = 4;
    // timers
    timeOut01 = 5000; // time before iddle
    timeOut02 = 3000; // time before showCapture
    timeOut03 = 1500; // time before grid
    timer01.setup(timeOut01, false), timer02.setup(timeOut02, false), timer03.setup(timeOut03, false);
    // ft
    focusTime = 10; // time before focusing + recording
    smoothingRate = 1;
    enableTracking = true;
    isFocused = false;
    faceAvgWidth = 0, faceAvgHeight = 0, faceTotalFrame = 0, faceTotalWidth = 0, faceTotalHeight = 0;
    // filter
    claheClipLimit = 2;
    inputIsFiltered = true, inputIsClaheColored = false, imgIsFiltered = true;
    // grid
    showGrid = false;
    showGridElements = false;
    gridWidth = 6, gridHeight = 6, gridRes = 32, gridMinSize = 0, gridMaxSize = 8;
    gridIsSquare = true;
    // ft capture
    faceElementsQty.assign(5,5);
    faceElementsOffset.assign(5,0);
    faceElementsZoom.assign(5,.5);
    // live
    volumes.assign(5,0), initTimes.assign(5,0), startVolumes.assign(5,0), endVolumes.assign(5,0);
    resetLive = true;
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
        ImGui::Checkbox("imgIsFiltered", &imgIsFiltered);
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
            ImGui::SliderInt("face Count", &faceElementsQty[0], 0, 20);
            ImGui::SliderInt("Left Eye Count", &faceElementsQty[1], 0, 20);
            ImGui::SliderInt("Right Eye Count", &faceElementsQty[2], 0, 20);
            ImGui::SliderInt("Nose Count", &faceElementsQty[3], 0, 20);
            ImGui::SliderInt("Mouth Count", &faceElementsQty[4], 0, 20);
//            ImGui::SliderFloat("Offset", &faceElementsOffset, 0, 10);
//            ImGui::SliderFloat("Zoom", &faceElementsZoom, 0.2, 6);
        }
    gui.end();
}



// LIVE
//--------------------------------------------------------------
void ofApp::initLive(){
    if (live.isLoaded()) {
        for ( int x=0; x<live.getNumTracks() ; x++ ){
            ofxAbletonLiveTrack *track = live.getTrack(x);
            track->setVolume(0);
        }
        live.setVolume(0.8);
        live.stop();
        live.play();
        live.setTempo(45);
        
    }
}


//--------------------------------------------------------------
void ofApp::refreshLive() {
    if (!live.isLoaded()) {
        return;
    }
    if (resetLive) {
        initLive();
        resetLive = false;
    }
    // change the different values
    auto duration = 3.f;
    for (int i=0; i<volumes.size(); i++) {
        if ( initTimes.at(i)!=0 ) {
            auto endTime = initTimes.at(i) + duration;
            auto now = ofGetElapsedTimef();
            volumes[i] = ofxeasing::map_clamp(now, initTimes[i], endTime, startVolumes[i], endVolumes[i], &ofxeasing::linear::easeIn);
        }
    }
    // change volumes
    for ( int i=0; i<live.getNumTracks() ; i++ ) live.getTrack(i)->setVolume(volumes[i]);


//    // Get tracks
//    ofxAbletonLiveTrack *track01 = live.getTrack(0); // Drums
//    ofxAbletonLiveTrack *track02 = live.getTrack(1); // Bass
//    ofxAbletonLiveTrack *track03 = live.getTrack(2); // Organ
//    ofxAbletonLiveTrack *track04 = live.getTrack(3); // Voices
//    ofxAbletonLiveTrack *track05 = live.getTrack(4); // Speech
//    ofxAbletonLiveTrack *track06 = live.getTrack(5); // Jazz Beat
//    // Volumes
//    track01->setVolume(volume[0]);
//    track02->setVolume(vol02);
//    track03->setVolume(vol03);
//    track04->setVolume(vol04);
//    track05->setVolume(vol05);
//    track06->setVolume(vol06);
//    // Fx - Melody
//    ofxAbletonLiveDevice *device = trackMelody->getDevice("Massive");
//    ofxAbletonLiveParameter *noiseColor = device->getParameter(1);
//    ofxAbletonLiveParameter *noiseAmp = device->getParameter(2);
//    ofxAbletonLiveParameter *dryWet = device->getParameter(3);
//    noiseColor->setValue(melodyRate01);
//    noiseAmp->setValue(melodyRate02);
//    dryWet->setValue(melodyRate01);
//    // Fx - Chords
//    ofxAbletonLiveDevice *device1 = trackChords->getDevice("Massive");
//    ofxAbletonLiveParameter *noiseColor1 = device1->getParameter(1);
//    ofxAbletonLiveParameter *noiseAmp1 = device1->getParameter(2);
//    ofxAbletonLiveParameter *cutOff = device1->getParameter(3);
//    ofxAbletonLiveParameter *resonance = device1->getParameter(4);
//    ofxAbletonLiveParameter *intensity = device1->getParameter(5);
//    noiseColor1->setValue(melodyRate01);  // reuse of same ofParameter not good
//    noiseAmp1->setValue(melodyRate02/4);
//    cutOff->setValue(chordsRate01);
//    resonance->setValue(chordsRate02);
//    intensity->setValue(chordsRate02/2);
//    // Fx - Bass
//    ofxAbletonLiveDevice *device2 = trackBass->getDevice("Massive");
//    ofxAbletonLiveParameter *cutOff1 = device2->getParameter(1);
//    ofxAbletonLiveParameter *resonance1 = device2->getParameter(2);
//    ofxAbletonLiveParameter *intensity1 = device2->getParameter(4);
//    cutOff1->setValue(bassRate01);
//    resonance1->setValue(bassRate01);
//    intensity1->setValue(bassRate02);
//    // Fx - Voice
//    ofxAbletonLiveDevice *device3 = trackVoice->getDevice("Reaktor 6 FX");
//    ofxAbletonLiveParameter *synth = device3->getParameter(2);
//    ofxAbletonLiveParameter *fx = device3->getParameter(5);
//    synth->setValue(voiceRate01);
//    fx->setValue(voiceRate01);
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
