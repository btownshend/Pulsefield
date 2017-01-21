#include <iostream>
#include "VisualizerJNI.h"
#include "ofApp.h"

using namespace std;

static testApp *app;
static int firstTime =1;
/*
 * Class:     VisualizerJNI
 * Method:    nsetup
 * Signature: (Lprocessing/core/PApplet;)V
 */
JNIEXPORT void JNICALL Java_VisualizerJNI_nsetup
(JNIEnv *, jobject, jobject) {
    cout << "nsetup" << endl;
}


/*
 * Class:     VisualizerJNI
 * Method:    nstart
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_VisualizerJNI_nstart
(JNIEnv *, jobject){
    cout << "nstart" << endl;
}

/*
 * Class:     VisualizerJNI
 * Method:    nstop
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_VisualizerJNI_nstop
(JNIEnv *, jobject) {
    cout << "nstop" << endl;
}

void initialize() {
    ofSetLogLevel(OF_LOG_VERBOSE);
    if (!glfwInit()) {
        cout << "glfw init failed" << endl;
    }
    cout << "glfw init done" << endl;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    cout << "glewInit done" << endl;
    GLuint err2 = glGetError(); //does this work?
    cout << "err=" << err2 << endl;
    app=new testApp();
    cout << "constructed testApp" << endl;
    app->setup();
    cout << "testApp setup done" << endl;
    firstTime=0;
}

/*
 * Class:     VisualizerJNI
 * Method:    nupdate
 * Signature: (Lprocessing/core/PApplet;LPeople;)V
 */
JNIEXPORT void JNICALL Java_VisualizerJNI_nupdate
(JNIEnv *, jobject, jobject, jobject) {
    //  cout << "nupdate" << endl;
    if (firstTime) {
        cout << "first update" << endl;
        initialize();
    }
    app->update();
}

/*
 * Class:     VisualizerJNI
 * Method:    ndraw
 * Signature: (LTracker;Lprocessing/core/PGraphics;LPeople;)V
 */
JNIEXPORT void JNICALL Java_VisualizerJNI_ndraw
(JNIEnv *, jobject, jobject, jobject, jobject) {
    //    cout << "ndraw" << endl;
    //     glClear(GL_COLOR_BUFFER_BIT);
    // glBegin(GL_POLYGON);
    //     glVertex3f(0.0, 0.0, 0.0);
    //     glVertex3f(0.5, 0.0, 0.0);
    //     glVertex3f(0.5, 0.5, 0.0);
    //     glVertex3f(0.0, 0.45, 0.0);
    // glEnd();
    // glFlush();
    if (firstTime) {
        cout << "first draw" << endl;
        initialize();
    }
    app->draw();
}
