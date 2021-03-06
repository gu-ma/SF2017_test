//
//  ofLogAudio.h
//  testOfxSpeech
//
//  Only working for Mac
//
//
// To do: auto assign voices, encoding for terminal view

#ifndef ofLogAudio_h
#define ofLogAudio_h

class ofLogAudio : public ofThread{
public:
    
    string voice, misc, msg, rawMsg, rawMsgProgress;
    vector<string> rawMsgWords;
    string pbas,pmod,rate,volm;
    string options;
    string currentWord;
    bool term;
    bool startSpeaking = false;
    bool randomVoice = false;
    const int defaultRate = 175;
    float startTime, endTime = 0;
    int wordIndex;
    
    void start() {
        startThread();
    }
    
    void LogAudio(string _voice, string _pbas, string _pmod, string _rate, string _volm, std::string _rawMsg, bool _term=false, string _misc="") {
        //
        term = _term;
        voice = _voice;
        pbas = _pbas;
        pmod = _pmod;
        rate = _rate;
        volm = _volm;
        rawMsg = _rawMsg;
        rawMsgProgress = "";
        wordIndex = -1;
        //
        options = "-v " + _voice + " " + _misc;
        msg = "[[pbas " + _pbas + "; pmod " + _pmod + "; rate " + _rate + "; volm " + _volm + "]] " + _rawMsg;
//        msg = _rawMsg;
        //
        startTime = ofGetElapsedTimef();
        rawMsgWords = ofSplitString(rawMsg, " ", false, true);
        float timePerWord = 45 / ofToFloat(rate); // Bad rate !!
        float timePerMsg = rawMsgWords.size() * timePerWord;
        endTime = startTime + timePerMsg;
//        cout << "" << endl;
//        cout << "number of words: " + ofToString(rawMsgWords.size()) << endl;
//        cout << "startTime: " + ofToString(startTime) << endl;
//        cout << "endTime: " + ofToString(endTime) << endl;
//        cout << "msg: " + rawMsg << endl;
//        cout << "" << endl;
        //
        startSpeaking = true;
    }
    
    void listVoices() {
        cout << ofSystem("say -v '?'") << endl;
    }
    
    void threadedFunction() {
        while(isThreadRunning()) {
            if(startSpeaking) {
                if (term) {
                    system("osascript -e 'tell application \"Terminal\" to activate'"); // Open Terminal
                    system("osascript -e 'tell application \"Terminal\" to get bounds of window 1'"); // Get bounds of Terminal on screen
                    system("osascript -e 'tell application \"Terminal\" to set bounds of window 1 to {10, 800, 600,400}'"); // Set new bounds of Terminal Window
                    system("osascript -e 'tell application \"System Events\" to set frontmost of the first process whose unix id is (do shell script \"ps aux | grep -v grep |grep -i terminal | awk \'{print $2;}\'\") to true'"); // focus
                    char myScript[255];
                    sprintf(myScript, "osascript -e 'tell application \"Terminal\" to do script \"say %s \" & quoted form of \"%s\" in front window'",options.c_str(),msg.c_str());
                    system(myScript);
                } else {
                    string cmd = "say " + options + " '" + msg + "' ";   // create the command
                    system(cmd.c_str());
                }
                startSpeaking = false;
            }
            
        }
    }
    
    bool speechUpdate() {
        int rawMsgWordsIndex = ofMap(ofGetElapsedTimef(), startTime, endTime, 0, rawMsgWords.size()-1);
        if( rawMsgWordsIndex < rawMsgWords.size() && rawMsgWordsIndex!=wordIndex){
            currentWord = rawMsgWords[rawMsgWordsIndex];
            wordIndex = rawMsgWordsIndex;
//            cout <<  "" +  ofToString(ofGetElapsedTimef()) + " - " + currentWord << endl;
            return true;
        } else {
            return false;
        }
    }
    
    string getCurrentWord() {
        return currentWord;
    }
};

#endif /* ofLogAudio_h */
