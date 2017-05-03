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
// make different grid types (GridElement Struct)
// animation when opening
// improve the "updatePixels" function
// improve the generate function (no random)
//


class ofGrid : public ofBaseApp {
    
public:

    enum ContentType { pixels, text };
    enum PixelsType { face, leftEye, rightEye, mouth, nose };
//    enum GridType { mosaic, random, big, small };
//    enum LayoutType { faceLike, mixed, repeat };

    //
    class PixelsItem {
    public:
        ofPixels pixels;
        PixelsType type;
        ofTexture tex;

        PixelsItem(ofPixels pix, PixelsType t): pixels(pix), type(t) {
            if (pix.isAllocated()) tex.loadData(pix);
        }

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
//    class TextItem {
//    public:
//        string textBuffer;
//        int scaling, fontSize, lineHeight, padding;
//        ofTrueTypeFont textDisplay;
//        
//        TextItem(string txt, int s): textBuffer(txt), scaling(s) {
//            this->lineHeight = 10;
//            this->fontSize = 6*s;
//            this->padding = 6*s;
//            textDisplay.load("fonts/pixelmix.ttf", this->fontSize, false, false, false, 144);
//            textDisplay.setLineHeight(this->lineHeight*s);
//        }
//        
//        void draw(int x, int y, int w, int h, ofColor bckgrdColor) {
//            if (!textBuffer.empty()) {
//                wrapString(w-this->padding*2);
//                ofSetColor(bckgrdColor);
//                ofDrawRectangle(x, y, w, h);
//                ofSetColor(255);
//                textDisplay.drawString(textBuffer, x+this->padding/2, y+this->padding+4*this->scaling);
////                textDisplay.drawString(textBuffer, x+3*scaling, y+4*scaling+(6*scaling));
//            }
//        }
//        
//        int getArea() {
//            int c = 1; // buffer to add for the char width
//            int area = textBuffer.size() * (this->textDisplay.getLineHeight()) * (this->textDisplay.getSize()+c);
//            int perimeterPadding = sqrt(area) * 4 * this->padding;
//            return (area + perimeterPadding);
//        }
//        
//        void wrapString(int width) {
//            if (!textBuffer.empty()) {
//                string typeWrapped = "";
//                string tempString = "";
//                vector <string> words = ofSplitString(textBuffer, " ");
//                for(int i=0; i<words.size(); i++) {
//                    string wrd = words[i];
//                    if (i > 0) {
//                        // if we aren't on the first word, add a space
//                        tempString += " ";
//                    }
//                    tempString += wrd;
//                    int stringwidth = textDisplay.stringWidth(tempString);
//                    if(stringwidth >= width) {
//                        typeWrapped += "\n";
//                        tempString = wrd;		// make sure we're including the extra word on the next line
//                    } else if (i > 0) {
//                        // if we aren't on the first word, add a space
//                        typeWrapped += " ";
//                    }
//                    typeWrapped += wrd;
//                }
//                textBuffer = typeWrapped;
//            }
//        }
//
//        void clear() {
//            textBuffer.clear();
//            scaling = 1;
//        }
//        
//    };

    class GridElement {
    public:
        ofRectangle rectangle;
        ContentType contentType;
        PixelsType pixelsType;
        int alpha;
        GridElement(ofRectangle rect, ContentType ct, PixelsType pt, int a): rectangle(rect), contentType(ct), pixelsType(pt), alpha(a){
        }
        void setAlpha(int a) {
            this->alpha = a;
        }
    };

    int width, height, resolution, minSize, maxSize;
    bool squareOnly;
    vector<PixelsItem> pixelsItems;
    vector<GridElement> GridElements;
    //    vector<TextItem> textItems;
    
    //
    void init(int width, int height, int resolution, int minSize, int maxSize, bool squareOnly) {
        this->width = width;
        this->height = height;
        this->resolution = resolution;
        this->minSize = minSize;
        this->maxSize = maxSize;
        this->squareOnly = squareOnly;
        this->clearGridElements();
        this->generateGridElements();
        this->clearPixels();
    }
    
    //
    void draw() {
        int i = 0;
        int j = 0;
        for (auto & ge : GridElements) {
            //
            int x = ge.rectangle.x*this->resolution;
            int y = ge.rectangle.y*this->resolution;
            int w = ge.rectangle.getWidth()*this->resolution;
            int h = ge.rectangle.getHeight()*this->resolution;
            //
            if (ge.contentType == pixels && !this->pixelsItems.empty()) {
//                ofSetColor( ofMap(ofNoise(x, y),0, 1, 0, 255), ofMap(ofNoise(y, x),0, 1, 255, 0), ofMap(ofNoise(ge.alpha),0, 1, 255, 0), ge.alpha);
//                ofSetColor( ofMap(x, 0, this->width*this->resolution, 0, 255), ofMap(y, 0, this->height*this->resolution, 0, 255), 255, ge.alpha);
                ofSetColor(255, ge.alpha);
                PixelsItem pc = this->pixelsItems.at((j)%this->pixelsItems.size());
                pc.cropAndDraw(x, y, w, h);
                j ++;
                ofSetColor(255);
            }
            //
//            if (ge.contentType == text && !this->textItems.empty()) {
//                TextItem ti = this->textItems.at((i)%this->textItems.size());
////                ti.draw(x, y, w, h, ofColor(ofMap(i, 0, this->textItems.size(), 0, 255), 0, 0) );
//                ti.draw(x, y, w, h, ofColor(255, 0, 0) );
//                i ++;
//            }
        }
        
    }
    
    void updatePixels(vector<PixelsItem> pi){
        this->pixelsItems.assign(pi.begin(), pi.end());
    }
    
    void clearPixels(){
        this->pixelsItems.clear();
    }
//
//    void updateText(vector<TextItem> ti){
//        this->textItems.assign(ti.begin(), ti.end());
//    }
//
//    void clearText(){
//        this->textItems.clear();
//    }
    
    void clearGridElements() {
        this->GridElements.clear();
    }
    
    void generateGridElements() {
        this->clearGridElements();
        int i = 0;
        int x,y,w,h,rw,rh;
        
//        // create text grid
//        while (!this->textItems.empty() && i<textItems.size()) {
//            x = ofRandom(this->width);
//            y = ofRandom(this->height);
//            w = sqrt(textItems.at(i).getArea())/this->resolution;
//            h = w;
////            if (rw <= (this->width-x) && rw <= (this->height-y)) {
////                w = rw;
////                h = rw;
////            } else {
////                w = (this->width-x > this->height-y) ? this->height-y : this->width-x;
////                h = w;
////            }
//            if (addGridElement(i, text, leftEye, x, y, w, h)) i++;
//        }
        
        // create pixel grid
        // sometime makes it following the grid exactly
        if (ofRandom(1)<.6 && this->width==6 && this->height==6 && this->resolution==32) {
            createGridTemplate(squareOnly); // 6*6 only
        } else {
            while (this->canAddGridElement()) {
                x = ofRandom(this->width);
                y = ofRandom(this->height);
                rw = ofRandom(this->maxSize)+1;
                if (squareOnly) {
                    if (rw <= (this->width-x) && rw <= (this->height-y)) {
                        w = rw;
                        h = rw;
                    } else {
                        w = (this->width-x > this->height-y) ? this->height-y : this->width-x;
                        h = w;
                    }
                } else {
                    w = (rw <= (this->width-x)) ? rw : this->width-x;
                    rh = ofRandom(this->maxSize)+1;
                    h = (rh <= (this->height-y)) ? rh : this->height-y;
                }
                if (addGridElement(i, pixels, leftEye, x, y, w, h)) i++;
            }
        }
    }
    
    //
    bool addGridElement(int index, ContentType ct, PixelsType pt, int x, int y, int w, int h) {
        bool intersects = false;
        ofRectangle rect = ofRectangle(x,y,w,h);
        for (auto & GridElement : GridElements) {
            if (rect.intersects(GridElement.rectangle)) intersects = true;
        }
        if (!intersects) {
            GridElement gh(rect,ct,pt,255);
            GridElements.push_back(gh);
            return true;
        } else {
            return false;
        }
    }

    void drawGridElements() {
        int i = 0;
        for (auto & GridElement : GridElements) {
            ofSetColor(ofColor(i*2));
            int x = GridElement.rectangle.x*this->resolution;
            int y = GridElement.rectangle.y*this->resolution;
            int w = GridElement.rectangle.getWidth()*this->resolution;
            int h = GridElement.rectangle.getHeight()*this->resolution;
            ofDrawRectangle(x,y,w,h);
            ofSetColor(ofColor(255));
            ofDrawBitmapString(i, x+5, y+15);
            i++;
        }
    }
    
    bool canAddGridElement() {
        int area = 0;
        for (auto & GridElement : GridElements) {
            area += GridElement.rectangle.getArea();
        }
        int gridArea = this->width*this->height;
        bool b = (area<gridArea) ? true : false;
        return b;
        
    }
    
    void createGridTemplate(bool isSquare) {
        if (isSquare) {
            // first row
            addGridElement(1, pixels, leftEye, 0, 0, 1, 1);
            addGridElement(2, pixels, leftEye, 0, 1, 1, 1);
            addGridElement(3, pixels, leftEye, 0, 2, 1, 1);
            addGridElement(4, pixels, leftEye, 0, 3, 1, 1);
            addGridElement(5, pixels, leftEye, 0, 4, 1, 1);
            addGridElement(6, pixels, leftEye, 0, 5, 1, 1);
            // 2nd row
            addGridElement(7, pixels, leftEye, 1, 0, 1, 1);
            addGridElement(8, pixels, leftEye, 1, 1, 1, 1);
            addGridElement(9, pixels, leftEye, 1, 2, 1, 1);
            addGridElement(10, pixels, leftEye, 1, 3, 2, 2);
            addGridElement(11, pixels, leftEye, 1, 5, 1, 1);
            // 3rd row
            addGridElement(12, pixels, leftEye, 2, 0, 2, 2);
            addGridElement(13, pixels, leftEye, 2, 2, 1, 1);
            addGridElement(14, pixels, leftEye, 2, 5, 1, 1);
            // 4th row
            addGridElement(15, pixels, leftEye, 3, 2, 1, 1);
            addGridElement(16, pixels, leftEye, 3, 3, 1, 1);
            addGridElement(17, pixels, leftEye, 3, 4, 1, 1);
            addGridElement(18, pixels, leftEye, 3, 5, 1, 1);
            // 5th row
            addGridElement(19, pixels, leftEye, 4, 0, 1, 1);
            addGridElement(20, pixels, leftEye, 4, 1, 1, 1);
            addGridElement(21, pixels, leftEye, 4, 2, 1, 1);
            addGridElement(22, pixels, leftEye, 4, 3, 1, 1);
            addGridElement(23, pixels, leftEye, 4, 4, 2, 2);
            // 6th row
            addGridElement(24, pixels, leftEye, 5, 0, 1, 1);
            addGridElement(25, pixels, leftEye, 5, 1, 1, 1);
            addGridElement(26, pixels, leftEye, 5, 2, 1, 1);
            addGridElement(27, pixels, leftEye, 5, 3, 1, 1);
        } else {
            // first row
            addGridElement(1, pixels, leftEye, 0, 0, 1, 1);
            addGridElement(2, pixels, leftEye, 0, 1, 1, 3);
            addGridElement(3, pixels, leftEye, 0, 4, 1, 1);
            addGridElement(4, pixels, leftEye, 0, 5, 4, 1);
            // 2nd row
            addGridElement(5, pixels, leftEye, 1, 0, 1, 2);
            addGridElement(6, pixels, leftEye, 1, 2, 3, 1);
            addGridElement(7, pixels, leftEye, 1, 3, 2, 2);
            // 3rd row
            addGridElement(8, pixels, leftEye, 2, 0, 2, 2);
            // 4th row
            addGridElement(9, pixels, leftEye, 3, 3, 2, 1);
            addGridElement(10, pixels, leftEye, 3, 4, 1, 1);
            // 5th row
            addGridElement(11, pixels, leftEye, 4, 0, 1, 2);
            addGridElement(12, pixels, leftEye, 4, 2, 1, 1);
            addGridElement(13, pixels, leftEye, 4, 4, 2, 2);
            // 6th row
            addGridElement(14, pixels, leftEye, 5, 0, 1, 1);
            addGridElement(15, pixels, leftEye, 5, 1, 1, 1);
            addGridElement(16, pixels, leftEye, 5, 2, 1, 2);
        }

    }
    
    
private:
    
};

#endif /* ofGrid_h */
