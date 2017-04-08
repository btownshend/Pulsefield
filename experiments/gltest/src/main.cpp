#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    if (0) {
        ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context
        
        // this kicks off the running of my app
        // can be OF_WINDOW or OF_FULLSCREEN
        // pass in width and height too:
        ofRunApp(new ofApp());
    } else {
        ofSetLogLevel(OF_LOG_VERBOSE);
        // Check current thread
        cout << "Current thread ismain=" << ofThread::isMainThread() << endl;
        //ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context
        ofGLWindowSettings settings;
        settings.width = 600;
        settings.height = 600;
        settings.setPosition(ofVec2f(300,0));
        auto mainWindow = ofCreateWindow(settings);
        auto mainApp = make_shared<ofApp>();
        ofRunApp(mainWindow, mainApp);
        ofRunMainLoop();
    }

}
