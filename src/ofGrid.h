//
//  ofGrid.h
//  example-FaceTracker
//
//  Created by Guillaume on 14.04.17.
//
//

#ifndef ofGrid_h
#define ofGrid_h

// DONE
// Crop
// TO DO
// make different grid types (gridHolder Struct)
// animation when opening
// improve the "updatePixels" function
// improve the generate function (no random)
//


class ofGrid : public ofBaseApp {
    
public:

    enum ContentType { pixels, text };
    enum PixelsType { face, leftEye, rightEye, mouth, nose };
    enum GridType { mosaic, random, big, small };
    enum LayoutType { faceLike, mixed, repeat };

    //
    class PixelsItem {
    public:
        ofPixels pixels;
        PixelsType type;
        ofTexture tex;

        PixelsItem(ofPixels pix, PixelsType t): pixels(pix), type(t) {
            if (pix.isAllocated()) tex.loadData(pix);
        }
        //        void init(ofPixels pix, PixelsType t){
        //            pixels = pix;
        //            type = t;
        //            tex.loadData(pix);
        //        }

        void cropAndDraw(int x, int y, int w, int h) {
            if (tex.isAllocated()) {
                float texW = tex.getWidth();
                float texH = tex.getHeight();
                float cropW, cropH, cropX, cropY;
                if (w/h > texW/texH) {
                    cropW = texW;
                    cropH = texW/(float(w)/float(h));
                    cropX = 0;
                    cropY = (texH-cropH)/2;
                } else {
                    cropW = texH*(float(w)/float(h));
                    cropH = texH;
                    cropX =(texW-cropW)/2;
                    cropY = 0;
                }
                tex.drawSubsection(x, y, w, h, cropX, cropY, cropW, cropH);
            }
        }
    };
    
    //
    class TextItem {
    public:
        string textBuffer;
        int size;
        ofTrueTypeFont textDisplay;
        
        TextItem(string txt, int s): textBuffer(txt), size(s) {
            textDisplay.load("fonts/pixelmix.ttf", 6*s, false, false, true, 144);
            textDisplay.setLineHeight(10*s);
        }
        
        void draw(int x, int y, int w, int h) {
            if (!textBuffer.empty()) {
                wrapString(w-3*size*2);
                ofSetColor(x/3, y/3, h/2);
                ofDrawRectangle(x, y, w, h);
                ofSetColor(255);
//                textDisplay.drawStringAsShapes(textBuffer, x, y);
                textDisplay.drawString(textBuffer, x+3*size, y+4*size+(6*size));
            }
        }
        
        void wrapString(int width) {
            if (!textBuffer.empty()) {
                string typeWrapped = "";
                string tempString = "";
                vector <string> words = ofSplitString(textBuffer, " ");
                for(int i=0; i<words.size(); i++) {
                    string wrd = words[i];
                    if (i > 0) {
                        // if we aren't on the first word, add a space
                        tempString += " ";
                    }
                    tempString += wrd;
                    int stringwidth = textDisplay.stringWidth(tempString);
                    if(stringwidth >= width) {
                        typeWrapped += "\n";
                        tempString = wrd;		// make sure we're including the extra word on the next line
                    } else if (i > 0) {
                        // if we aren't on the first word, add a space
                        typeWrapped += " ";
                    }
                    typeWrapped += wrd;
                }
                textBuffer = typeWrapped;
            }
        }

        void clear() {
            textBuffer.clear();
            size = 1;
        }
        
    };

    class GridHolder {
    public:
        ofRectangle rectangle;
        ContentType contentType;
        PixelsType pixelsType;
        int alpha;
        GridHolder(ofRectangle rect, ContentType ct, PixelsType pt, int a): rectangle(rect), contentType(ct), pixelsType(pt), alpha(a){
        }
    };

    int width, height, resolution, minSize, maxSize;
    bool squareOnly;
    vector<PixelsItem> pixelsItems;
    vector<TextItem> textItems;
    vector<GridHolder> gridHolders;
    
    //
    void init(int width, int height, int resolution, int minSize, int maxSize, bool squareOnly) {
        this->width = width;
        this->height = height;
        this->resolution = resolution;
        this->minSize = minSize;
        this->maxSize = maxSize;
        this->squareOnly = squareOnly;
        this->clearGridHolders();
        this->generateGridHolders();
        this->clearPixels();
    }
    
    //
    void draw() {
        int i = 0;
        for (auto & gridHolder : gridHolders) {
            //
            int x = gridHolder.rectangle.x*this->resolution;
            int y = gridHolder.rectangle.y*this->resolution;
            int w = gridHolder.rectangle.getWidth()*this->resolution;
            int h = gridHolder.rectangle.getHeight()*this->resolution;
            //
            if (!this->pixelsItems.empty()) {
                PixelsItem pc = this->pixelsItems.at((i)%this->pixelsItems.size());
//                ofSetColor(255, ofNoise(w, h)*255);
                pc.cropAndDraw(x, y, w, h);
            }
//            if (!this->textItems.empty()) {
//                TextItem ti = this->textItems.at((i)%this->textItems.size());
//                ti.draw(x, y, w, h);
//            }
            i++;
        }
    }
    
    void updatePixels(vector<PixelsItem> pi){
        this->pixelsItems.assign(pi.begin(), pi.end());
    }
    
    void clearPixels(){
        this->pixelsItems.clear();
    }

    void updateText(vector<TextItem> ti){
        this->textItems.assign(ti.begin(), ti.end());
    }

    void clearText(){
        this->textItems.clear();
    }
    
    void clearGridHolders() {
        this->gridHolders.clear();
    }
    
    void generateGridHolders() {
        this->clearGridHolders();
        int i = 0;
        int x,y,w,h,randomWidth,randomHeight;
        for (auto & ti : textItems) {
            
        }
        while (this->canAddGridHolder()) {
            x = ofRandom(this->width);
            y = ofRandom(this->height);
            randomWidth = ofRandom(this->maxSize)+1;
            if (squareOnly) {
                if (randomWidth <= (this->width-x) && randomWidth <= (this->height-y)) {
                    w = randomWidth;
                    h = randomWidth;
                } else {
                    w = (this->width-x > this->height-y) ? this->height-y : this->width-x;
                    h = w;
                }
            } else {
                w = (randomWidth <= (this->width-x)) ? randomWidth : this->width-x;
                randomHeight = ofRandom(this->maxSize)+1;
                h = (randomHeight <= (this->height-y)) ? randomHeight : this->height-y;
            }
            if (addGridHolder(i, pixels, leftEye, x, y, w, h)) {
                i++;
            }
        }
    }
    
    //
    bool addGridHolder(int index, ContentType ct, PixelsType pt, int x, int y, int w, int h) {
        bool intersects = false;
        ofRectangle rect = ofRectangle(x,y,w,h);
        for (auto & gridHolder : gridHolders) {
            if (rect.intersects(gridHolder.rectangle)) intersects = true;
        }
        if (!intersects) {
            GridHolder gh(rect,ct,pt,100);
            gridHolders.push_back(gh);
            return true;
        } else {
            return false;
        }
    }

    void drawGridHolders() {
        int i = 0;
        for (auto & gridHolder : gridHolders) {
            ofSetColor(ofColor(i*2));
            int x = gridHolder.rectangle.x*this->resolution;
            int y = gridHolder.rectangle.y*this->resolution;
            int w = gridHolder.rectangle.getWidth()*this->resolution;
            int h = gridHolder.rectangle.getHeight()*this->resolution;
            ofDrawRectangle(x,y,w,h);
            ofSetColor(ofColor(255));
            ofDrawBitmapString(i, x+5, y+15);
            i++;
        }
    }
    
    bool canAddGridHolder() {
        int area = 0;
        for (auto & gridHolder : gridHolders) {
            area += gridHolder.rectangle.getArea();
        }
        int gridArea = this->width*this->height;
        bool b = (area<gridArea) ? true : false;
        return b;
        
    }
    
    
private:
    
//private:
//
//    vector<vector<int>> matrix;
//
//    void initMatrix(){
//        for (int y = 0; y < this->height; y++) {
//            vector<int> row; // Create an empty row
//            for (int x = 0; x < this->width; x++) {
//                row.push_back(y*x); // Add an element (column) to the row
//            }
//            this->matrix.push_back(row); // Add the row to the main vector
//        }
//    }
//    
//    void clearMatrix(){
//        for (int y = 0; y < this->height; y++) {
//            for (int x = 0; x < this->width; x++) {
//                this->matrix[y][x] = 0;
//            }
//        }
//    }
//    
//    void printMatrix(){
//        cout << "----------" << endl;
//        for (int y = 0; y < this->height; y++) {
//            for (int x = 0; x < this->width; x++) {
//                cout << ofToString(this->matrix[y][x]) + " ";
//            }
//            cout << endl;
//        }
//    }
//    
//    bool isMatrixFull(){
//        bool isFull = true;
//        for (int y = 0; y < this->height; y++) {
//            for (int x = 0; x < this->width; x++) {
//                if (this->matrix[y][x]==0) isFull = false;
//            }
//        }
//        return isFull;
//    }
//    
//    void createMatrix() {
//        int i = 1;
//        cout << this->isMatrixFull() << endl;
//        
//        int x,y,w,h,randomWidth,randomHeight;
//        while (!this->isMatrixFull()) {
//            x = ofRandom(this->width);
//            y = ofRandom(this->height);
//            randomWidth = ofRandom(this->maxSize)+1;
//            if (squareOnly) {
//                if (randomWidth <= (this->width-x) && randomWidth <= (this->height-y)) {
//                    w = randomWidth;
//                    h = randomWidth;
//                } else {
//                    w = (this->width-x > this->height-y) ? this->height-y : this->width-x;
//                    h = w;
//                }
//            } else {
//                w = (randomWidth <= (this->width-x)) ? randomWidth : this->width-x;
//                randomHeight = ofRandom(this->maxSize);
//                h = (randomHeight <= (this->height-y)) ? randomHeight : this->height-y;
//            }
//            if (addMatrixElement(i,x,y,w,h)) {
//                cout << "w:" + ofToString(w) + " i:" + ofToString(i) + " x:" + ofToString(x) << endl;
////                this->printMatrix();
//                i++;
//            }
//
//        }
//    }
//    
//    bool addMatrixElement(int index, int x, int y, int w, int h) {
//        bool isEmpty = true;
//        for (int i=y; i<y+h; i++) {
//            for (int j=x; j<x+w; j++) {
//                if (this->matrix[i][j]!=0) isEmpty = false;
//            }
//        }
//        if (isEmpty) {
//            for (int i=y; i<y+h; i++) {
//                for (int j=x; j<x+w; j++) {
//                    this->matrix[i][j]=index;
//                }
//            }
//            return true;
//        } else {
//            return false;
//        }
//    }
    
};

#endif /* ofGrid_h */
