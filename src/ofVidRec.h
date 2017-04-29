//
//  vidRecorder.h
//  SF2017-FaceTracker_Capture_test
//
//  Created by Guillaume on 19.04.17.
//
//

#ifndef ofVidRec_h
#define ofVidRec_h

class ofVidRec : public ofBaseApp {
    
public:
    
    ofxVideoRecorder recorder;
    
    string filePath, fileName, fileExt, codec, bitrate;
    int width, height, frameRate, sampleRate, channels;
    bool isRecording;

    //--------------------------------------------------------------
    void init(string fe, string co, string br) {
        this->fileExt = fe;
        this->codec = co;
        this->bitrate = br;
        this->sampleRate = 44100;
        this->channels = 2;
        
        this->isRecording = false;
        this->recorder.setVideoCodec(codec);
        this->recorder.setVideoBitrate(bitrate);
//        vidRecorder.setAudioCodec("mp3");
//        vidRecorder.setAudioBitrate("320k");
        ofAddListener(this->recorder.outputFileCompleteEvent, this, &ofVidRec::complete);
    }
    
    //--------------------------------------------------------------
    void update(ofPixels pix){
        if (this->isRecording) {
            pix.resize(this->width, this->height);
            bool success = this->recorder.addFrame(pix);
            if (!success) {
                ofLogWarning("This frame was not added!");
            }
        }
        
        // Check if the video recorder encountered any error while writing video frame or audio smaples.
        if (this->recorder.hasVideoError()) {
            ofLogWarning("The video recorder failed to write some frames!");
        }
        
        if (this->recorder.hasAudioError()) {
            ofLogWarning("The video recorder failed to write some audio samples!");
        }
    }

    //--------------------------------------------------------------
    void start(string fp, string fn, int w, int h, int fr){
        this->filePath = fp;
        this->fileName = fn;
        this->width = w;
        this->height = h;
        this->frameRate = fr;
        this->isRecording = !this->isRecording;
        if(this->isRecording && !this->recorder.isInitialized()) {
            this->recorder.setup(this->filePath + "/" + ofGetTimestampString("%Y%m%d-%H%M%S") + "-" + this->fileName + this->fileExt, this->width, this->height, this->frameRate, true, false); // no audio
            this->recorder.start();
        }
    }

    //--------------------------------------------------------------
    void stop(){
        this->isRecording = false;
        this->recorder.close();
    }

    //--------------------------------------------------------------
    void pause(){
        if (this->isRecording && this->recorder.isInitialized()) {
            this->recorder.setPaused(true);
        }
        else if (this->isRecording && this->recorder.isInitialized()) {
            this->recorder.setPaused(false);
        }
    }
    
    //--------------------------------------------------------------
    void complete(ofxVideoRecorderOutputFileCompleteEventArgs& args){
        cout << "The recoded video file is now complete." << endl;
    }

    //--------------------------------------------------------------
    void close() {
        ofRemoveListener(this->recorder.outputFileCompleteEvent, this, &ofVidRec::complete);
        this->recorder.close();
    }
    
};

#endif /* ofVidRec_h */
