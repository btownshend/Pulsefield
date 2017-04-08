//
//  JNI_stub.cpp
//  gltest
//
//  Created by Brent Townshend on 1/20/17.
//
//


#include <iostream>
#include "JNI_stub.h"
#include "ofApp.h"

using namespace std;

static int firstTime =1;


void initialize() {
    ofSetLogLevel(OF_LOG_VERBOSE);
    // Check current thread
    cout << "Current thread ismain=" << ofThread::isMainThread() << endl;
    //ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context
    ofGLWindowSettings settings;
    settings.width = 600;
    settings.height = 600;
    settings.setPosition(ofVec2f(300,0));
    auto mainWindow = ofCreateWindow(settings);
    cout << "Created window" << endl;
    auto mainApp = make_shared<ofApp>();
    ofRunApp(mainWindow, mainApp);
//    if (!glfwInit()) {
//        cout << "glfw init failed" << endl;
//    }
//    cout << "glfw init done" << endl;
//    GLenum err = glewInit();
//    if (GLEW_OK != err)
//    {
//        /* Problem: glewInit failed, something is seriously wrong. */
//        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
//    }
//    cout << "glewInit done" << endl;
//    GLuint err2 = glGetError(); //does this work?
//    cout << "err=" << err2 << endl;
//    app=new ofApp();
//    cout << "constructed ofApp" << endl;
//    app->setup();
//    cout << "ofApp setup done" << endl;
//    firstTime=0;
}

/*
 * Class:     JNI_stub
 * Method:    nsetup
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_JNI_1stub_nsetup
(JNIEnv *, jobject) {
    cout << "nsetup" << endl;
   // initialize();
}


/*
 * Class:     JNI_stub
 * Method:    nupdate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_JNI_1stub_nupdate
(JNIEnv *, jobject) {
    //  cout << "nupdate" << endl;
    if (firstTime) {
        cout << "first update" << endl;
        initialize();
    }
    ofGetMainLoop()->loopOnce();
}

/*
 * Class:     JNI_stub
 * Method:    ndraw
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_JNI_1stub_ndraw
(JNIEnv *, jobject) {
    //    cout << "ndraw" << endl;

    if (firstTime) {
        cout << "first draw" << endl;
        initialize();
    }
    ofGetMainLoop()->loopOnce();
}
