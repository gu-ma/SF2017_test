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
        cam.setDeviceID(1);
        cam.setup(1920, 1080);
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
        colorImg.resize(colorImg.getWidth()/downSize, colorImg.getHeight()/downSize);
        if (isFiltered) {
            clahe.filter(colorImg, filteredImg, claheClipLimit, isClaheColored);
            filteredImg.update();
            ft.findFaces(filteredImg.getPixels(),false);
        } else {
            ft.findFaces(colorImg.getPixels(),false);
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
                        pis.push_back(*new ofGrid::PixelsItem(getFacePart(vidPixels, facePolyline, faceElementsZoom, i*faceElementsOffset, false), ofGrid::rightEye));
//                            pis.push_back(*new ofGrid::PixelsItem(getFacePart(vidPixels, facePolyline, ofMap(i,0,faceElementsCount.at(j),1,1.5)*faceElementsZoom, i*faceElementsOffset, true), ofGrid::rightEye));
                    }
                    j++;
                }
//                    ofGrid::PixelsItem pi(getFacePart(vidPixels, ofPolyline::fromRectangle(face.rect), 1, 0, true), ofGrid::leftEye);
//                    pis.push_back(pi);
            }
            // Capture
            if (i==0) {
                if (vidPixels.isAllocated()) {
//                    vidPixels.crop(face.noseTip.getCentroid2D().x-200, face.noseTip.getCentroid2D().y-200, 400, 400);
                    vidRecorder.update(getFacePart(vidPixels, ofPolyline::fromRectangle(face.rect), .5, 0, true));
                }
            }
        }
        grid.updatePixels(pis);
        // grid txt
        string t = "LOREM IPSUM DOLOR SIT AMET, CONSETETUR SADIPSCING ELITR, SED DIAM NONUMY EIRMOD TEMPOR INVIDUNT";
        for (int i=1; i<4; i++) {
            ofGrid::TextItem ti(t, i);
            tis.push_back(ti);
            ti.clear();
        }
        grid.updateText(tis);
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    if (showGrid) grid.drawGridHolders();
    else grid.draw();
    ofPushMatrix();
        ofTranslate(grid.width*grid.resolution, 0);
        ofScale(0.25, 0.25);
        if (isFiltered) filteredImg.draw(0, 0);
        else colorImg.draw(0, 0);
        ft.draw();
    ofPopMatrix();
    //
    guiDraw();
}

//--------------------------------------------------------------
ofPixels ofApp::getFacePart(ofPixels facePixels, ofPolyline partPolyline, float zoom, float offset, bool isSquare){
//    ofPixels crop;
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
    // check out of bound
    int x = center.x+offset-w/2;
    int y = center.y+offset-h/2;
    if ( x+w/2 > colorImg.getWidth() ) x = colorImg.getWidth()-w/2;
    else if ( x-w/2 < 0 ) x = w/2;
    if ( y+h/2 > colorImg.getHeight() ) y = colorImg.getHeight()-h/2;
    else if ( y-h/2 < 0 ) y = h/2;
    
    facePixels.crop(x, y, w, h);
//    facePixels.crop(center.x+ofSignedNoise(offset)*offset*10-w/2, center.y+ofSignedNoise(offset)*offset*10-h/2, w, h);
    return facePixels;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'h') grid.generateGridHolders();
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
    isClaheColored = true;
    // grid
    showGrid = false;
    gridWidth = 6;
    gridHeight = 6;
    gridRes = 32;
    gridScale = 2;
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
            ImGui::SliderInt("gridWidth", &gridWidth, 1, 12);
            ImGui::SliderInt("gridHeight", &gridHeight, 1, 12);
            ImGui::SliderInt("gridScale", &gridScale, 1, 12);
//            ImGui::SliderInt("gridMinSize", &gridMinSize, 0, 12);
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
            ImGui::SliderFloat("Zoom", &faceElementsZoom, 0.5, 6);
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
