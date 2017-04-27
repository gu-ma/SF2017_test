#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // general
    // ofSetBackgroundAuto(false);
    ofSetBackgroundColor(0);
    varSetup();
    // GUI
    gui.setup();
    // ft
    ft.setup("../../../../models/shape_predictor_68_face_landmarks.dat");
    ft.setDrawStyle(ofxDLib::lines);
    ft.setSmoothingRate(1);
    // grid
    grid.init(gridWidth, gridHeight, gridRes*gridScale, gridMinSize, gridMaxSize, true);
    // video recording
    vidRecorder.init("output", "face", ".mov", "mpeg4", "100k");
    // capture
    #ifdef _USE_LIVE_VIDEO
        cam.setDeviceID(0);
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
            colorImg.setFromPixels(cam.getPixels());
        #else
            colorImg.setFromPixels(movie.getPixels());
        #endif
        colorImg.mirror(false, true);
        colorImg.crop( (colorImg.getWidth()-colorImg.getHeight())/2, 0, colorImg.getHeight(), colorImg.getHeight() );
        // resize
        resizedImg = colorImg;
        resizedImg.resize(resizedImg.getWidth()/downSize, resizedImg.getHeight()/downSize);
        // ft
        if (isFiltered) {
            clahe.filter(resizedImg, filteredImg, claheClipLimit, isClaheColored);
            filteredImg.update();
            clahe.filter(colorImg, colorImg, claheClipLimit, isClaheColored);
            colorImg.update();
            ft.findFaces(filteredImg.getPixels(),false);
        } else {
            ft.findFaces(resizedImg.getPixels(),false);
        }
        vector<ofxDLib::Face> faces = ft.getFaces();
        ofPixels vidPixels = colorImg.getPixels();
        // grid
        vector<ofGrid::PixelsItem> pis;
        vector<ofGrid::TextItem> tis;
        int i = 0;
        for (auto & face : faces) {
            for (int i=0; i<20; i++) {
                vector<ofPolyline> facePolylines {ofPolyline::fromRectangle(face.rect), face.leftEye, face.rightEye, face.noseTip, face.outerMouth};
                int j = 0;
                for (auto & facePolyline : facePolylines) {
                    if (i < faceElementsCount.at(j)) {
                        // WARNING WITH MEMORY
                        // Add scale for the zoom and offset
                         pis.push_back(ofGrid::PixelsItem(getFacePart(vidPixels, facePolyline, faceElementsZoom, i*faceElementsOffset, true), ofGrid::rightEye));
                    }
                    j++;
                }
            }
            // video recording
            if (i==0) {
                if (vidPixels.isAllocated()) {
                    vidRecorder.update(getFacePart(vidPixels, ofPolyline::fromRectangle(face.rect), .5, 0, true));
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
    if (showGrid) grid.drawGridElements();
    else grid.draw();
    ofPushMatrix();
        ofTranslate(grid.width*grid.resolution, 0);
        ofScale(0.25, 0.25);
        if (isFiltered) filteredImg.draw(0, 0);
        else resizedImg.draw(0, 0);
        ft.draw();
    ofPopMatrix();
    //
    guiDraw();
}


//--------------------------------------------------------------
ofPixels ofApp::getFacePart(ofPixels facePixels, ofPolyline partPolyline, float zoom, float offset, bool isSquare){
    ofPoint center = partPolyline.getCentroid2D();
    int w,h;
    if (isSquare) {
        if ( partPolyline.getBoundingBox().width > partPolyline.getBoundingBox().height ) {
            w = partPolyline.getBoundingBox().width * 1/zoom;
        } else {
            w = partPolyline.getBoundingBox().height * 1/zoom;
        }
        h = w;
    } else {
        w = partPolyline.getBoundingBox().width * 1/zoom;
        h = partPolyline.getBoundingBox().height * 1/zoom;
    }
    // check if out of bound
    int x = center.x+offset-w/2;
    int y = center.y+offset-h/2;
    if ( x+w/2 > resizedImg.getWidth() ) x = resizedImg.getWidth()-w/2;
    else if ( x-w/2 < 0 ) x = w/2;
    if ( y+h/2 > resizedImg.getHeight() ) y = resizedImg.getHeight()-h/2;
    else if ( y-h/2 < 0 ) y = h/2;
    //
    facePixels.crop(x*downSize, y*downSize, w*downSize, h*downSize);
    return facePixels;
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'h') grid.generateGridElements();
    if (key == 'g') showGrid = !showGrid;
    if (key == 'r') vidRecorder.start(256, 256, (int)ofGetFrameRate());
    if (key == 's') vidRecorder.stop();
}


//--------------------------------------------------------------
void ofApp::varSetup(){
    // capture
    downSize = 2;
    // filter
    claheClipLimit = 2;
    isFiltered = true;
    isClaheColored = false;
    // grid
    showGrid = false;
    gridWidth = 12;
    gridHeight = 12;
    gridRes = 16;
    gridScale = 1;
    gridMinSize = 0;
    gridMaxSize = 4;
    gridIsSquare = true;
    // ft capture
    faceElementsCount.assign(5, 5);
    faceElementsOffset = 0;
    faceElementsZoom = 1;
}


//--------------------------------------------------------------
void ofApp::guiDraw(){
    // GUI
    gui.begin();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::SliderFloat("Down size input", &downSize, 1, 4);
        ImGui::SliderInt("claheClipLimit", &claheClipLimit, 0, 6);
        ImGui::Checkbox("isFiltered", &isFiltered);
        ImGui::Checkbox("isClaheColored", &isClaheColored);
        if (ImGui::CollapsingHeader("Grid", false)) {
            ImGui::SliderInt("gridWidth", &gridWidth, 1, 24);
            ImGui::SliderInt("gridHeight", &gridHeight, 1, 24);
            ImGui::SliderInt("gridScale", &gridScale, 1, 12);
            ImGui::InputInt("gridRes", &gridRes, 8);
            ImGui::SliderInt("gridMaxSize", &gridMaxSize, 1, 12);
            ImGui::Checkbox("gridIsSquare", &gridIsSquare);
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
