/*
 *  ofxFluid.h
 *
 *  Created by Patricio Gonzalez Vivo on 9/29/11.
 *  Copyright 2011 http://PatricioGonzalezVivo.com All rights reserved.
 *
 *  Created ussing:
 *
 *    - Mark Harris article from GPU Gems 1
 *      http://http.developer.nvidia.com/GPUGems/gpugems_ch38.html
 *
 *    - Phil Rideout
 *      http://prideout.net/blog/?p=58
 *  
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 *  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *  OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  ************************************************************************************ 
 * 
 *  tex0 -> obstacles
 *
 */
 
#ifndef OFXFLUID
#define OFXFLUID

#include "ofMain.h"
#include "ofxFXObject.h"

class punctualForce  {
public:
    ofFloatColor color;
    ofVec2f pos;
    ofVec2f vel;
    float   rad;
    float   temp;
    float   den;
    punctualForce(ofPoint _pos, ofPoint _dir, ofFloatColor _col, float _rad = 1.0f, float _temp = 10.f, float _den = 1.f );
};

class ofxFluid : public ofxFXObject {
public:
    ofxFluid();
    
    void    allocate(int _width, int _height, float _scale = 0.5, bool _hd = true);
    
    void    setGravity(ofPoint _force){ gForce = _force; };
    ofPoint getGravity() const { return gForce; }
    void    setUseObstacles(bool _do);
    void    setObstacles(ofBaseHasTexture &_tex);
    
    void    addColor(ofTexture &_tex, float _pct = 1.0);
    void    addColor(ofBaseHasTexture &_tex, float _pct = 1.0);
    void    addVelocity(ofTexture &_tex, float _pct = 1.0);
    void    addVelocity(ofBaseHasTexture &_tex, float _pct = 1.0);
    void    addTemporalForce(ofPoint _pos, ofPoint _dir, ofFloatColor _col, float _rad = 1.0f, float _temp = 10.f, float _den = 1.f )
        { temporalForces.push_back(punctualForce(_pos * scale,_dir,_col,_rad,_temp,_den)); }
    void    addConstantForce(ofPoint _pos, ofPoint _dir, ofFloatColor _col, float _rad = 1.0f, float _temp = 10.f, float _den = 1.f )
        { constantForces.push_back(punctualForce(_pos * scale,_dir,_col,_rad,_temp,_den)); }
    void clearConstantForces() {
        constantForces.clear();
    }
    void setConstantForces(vector<punctualForce> forces) {
        constantForces=forces;
    }
    
    virtual ofTexture & getTexture() {
        return pingPong.src->getTexture();
    };
    
    ofTexture & getVelocityTexture() {
        return velocityBuffer.src->getTexture();
    };
    
    ofTexture & getTemperatureTexture() {
        return temperatureBuffer.src->getTexture();
    };
    
    ofTexture & getPressureTexture() {
        return pressureBuffer.src->getTexture();
    };
    
    //ofTexture & getTexture();
    const ofTexture & getTexture() const;
    
    void    clear(float _alpha = 1.0);
    void    clearAlpha();

    void    update();
    
    void    draw(int x = 0, int y = 0, float _width = -1, float _height = -1);
    void    drawVelocity(int x = 0, int y = 0, float _width = -1, float _height = -1);

    float   dissipation;
    float   velocityDissipation;
    float   temperatureDissipation;
    float   pressureDissipation;
    float   viscosity;  // Parameter of diffusion of velocity field
    float   diffusion;  // Diffusion of pingpong (dye) 
    int     numJacobiIterations;
    Boolean smokeEnabled;
    float   smokeBuoyancy;
    float   smokeWeight;
    float   ambientTemperature;
    
    float   scale;  // Internal grid size is scale*(width,height)
private:
    void    advect(ofxSwapBuffer& _buffer, float _dissipation);
    void    jacobi();
    void    viscousDiffusion(ofxSwapBuffer& _buffer, float _viscosity);
    void    subtractGradient();
    void    computeDivergence();
    void    project(); // Project onto zero-divergence solution
    void    applyImpulse(ofxSwapBuffer& _buffer, ofBaseHasTexture &_baseTex, float _pct = 1.0, bool _isVel = false);
    void    applyImpulse(ofxSwapBuffer& _buffer, ofPoint _force, ofPoint _value, float _radio = 3.f);
    void    applyImpulse(ofxSwapBuffer& _buffer, ofPoint _force, ofFloatColor _value, float _radio);
    void    applyBuoyancy();

    ofShader jacobiShader;
    ofShader subtractGradientShader;
    ofShader computeDivergenceShader;
    ofShader applyImpulseShader;
    ofShader applyTextureShader;
    ofShader applyBuoyancyShader;
    ofShader diffusionShader;
    
    ofxSwapBuffer  velocityBuffer;
    ofxSwapBuffer  temperatureBuffer;
    ofxSwapBuffer  pressureBuffer;
    
    ofFbo   divergenceFbo;
    ofFbo   obstaclesFbo;
    
    vector<punctualForce> constantForces;
    vector<punctualForce> temporalForces;
    ofPoint gForce;
    

    float   gradientScale;
    float   gridWidth,gridHeight;
    float   timeStep;
    float   cellSize;
    
    bool    bObstacles;
    
    ofFbo   colorAddFbo, velocityAddFbo;
    float   colorAddPct, velocityAddPct;
    
    int     colorGlFormat;
};
#endif
