//
//  clahe.h
//  CLAHE_test
//
//  Created by Guillaume on 22.04.17.
//
//

#ifndef clahe_h
#define clahe_h

class Clahe {
public:
    cv::Mat grey_img, lab_img, clahe_img, tmp_img;
    
    template <class S, class D>
    void filter(S& src, D& dst, int clip, bool isColor) {
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
        clahe->setClipLimit(clip);
        if (!isColor) {
            ofxCv::copyGray(src, grey_img);
            clahe->apply(grey_img,clahe_img);
            // convert to ofImage
            ofxCv::toOf(clahe_img, dst);
        } else {
            cv::cvtColor(ofxCv::toCv(src), lab_img, CV_BGR2Lab);
            // ofxCv::convertColor(camera, lab_img, CV_BGR2Lab);
            
            // Extract the L channel
            vector<cv::Mat> lab_planes(3);
            cv::split(lab_img, lab_planes);  // now we have the L image in lab_planes[0]
            
            // apply the CLAHE algorithm to the L channel
            clahe->apply(lab_planes[0], tmp_img);
            
            // Merge the the color planes back into an Lab img
            tmp_img.copyTo(lab_planes[0]);
            cv::merge(lab_planes, lab_img);
            
            // convert back to RGB
            cv::cvtColor(lab_img, clahe_img, CV_Lab2BGR);
            
            // convert to ofImage
            ofxCv::toOf(clahe_img, dst);
        }
    }
};

#endif /* clahe_h */
