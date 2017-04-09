# Introduction
The Pulsefield software includes the capability of adding new applications that make use of the people-tracking, multi-projector display and music/audio subsystems.  This can be done by adding a new class to the Java *Tracker* application as described below. Apps can be run and tested in a simulation mode where the output is shown in a window and the mouse can be used to simulate movement of people in the space.

# Setup
To install and setup the software:

1.  Clone the pulsefield repository from github into a local folder, "pulsefield", using:
    `git clone https://github.com/btownshend/Pulsefield.git pulsefield`
2. Install Eclipse for Java if you don't already have it.  Other IDEs or a plain editor can also be used, but the setup in the repository is for Eclipse.  I've been using Eclipse Neon version 3 with java JDK 1.8.0_102.
3. Set an environment variable, PFROOT, the root directory of the local copy (in this example, the full path to "pulsefield"). This can be done at the system level or in Eclipse's run configurations in the "Environment" tab.
4. Open Eclipse and use Import>General>Existing Projects into Workspace and import $PFROOT/modules/Tracker into the workspace.

# Source Tree
The Pulsefield repository will setup the following src tree:

    $PFROOT/	   		- root of install
	.doc/		  	 	- documentation
    ..AppDevelopment.md - this document
	..PROTOCOL.txt	   	- details of the OSC protocols used between subsystems
	.modules/	   		- high-level modules for Pulsefield
	..Tracker/	   		- Java-based software to generate Pulsefield apps
	..FrontEnd/   		- people-tracking system; sends OSC messages to
		Tracker and other subsystems (C++)
	..Calibrator/ 		- module for calibration of Pulsefield (C++)
	..Chuck/	   		- music-module to support the "chuck" app in Tracker
	..GPUFluid/   		- GPU-based code for Navier-Stokes simulation to support the "NavierOF" app in Tracker
	.lib/
	..C++Util	  		- common code for some of the C++ sbsystems
	.config/	   		- configuration files
	..urlconfig.txt 	- configuration of IP addresses and ports of subsystems
	.OSC/		 		- software relation to Open Sound Control (OSC)
	..TouchOSC	 		- templates for iPad control of Pulsefield via
    TouchOSC
	.RUN/		 		- setup for launching and running Pulsefield systems
	.Matlab/	 		- Matlab utilities, primarily for monitoring
		performance of tracking 

For doing application development, only the modules/Tracker folder is
relevant.  Apps can be run and tested standalone without using any of
the other subsystems.  The only exception is if the app uses real-time
control via TouchOSC, in which case the OSC/TouchOSC templates can be
editted using [TouchOSC Editor](https://hexler.net/docs/touchosc).

# Adding a new app
Applications within Tracer are implemented as classes derived from
Visualizer, typically named VisualizerXXX.  There are numerous
examples of different apps within the code, and an empty template
called VisualizerTemplate that can be copied.  Each of these implements
the following interface:

    constructor - called once when the system is started up;  there
    will only be a singleton of each app constructed by the system.
    
    start() - called each time the app becomes active
    
    stop() - called each time the app is deactivated
    
    update() - update internal state based on passed-in location of
    all the people currently in the pulsefield
	
	draw() - called at the refresh frame rate (~20/second).  This is
    where the app should update the display to be imaged onto the
    ground by drawing into a canvas passed in.

Drawing is based on Processing drawing calls.  A PGraphics instance is
passed in and drawing can be done using any [processing](https://processing.org/reference/) calls.

The location and other information about each person in the Pulsefield
is passed into update() and draw() in a class called People.  See the
code for details of the available methods.

To register a new app with the system, a small piece of code needs to
also be added to Tracker.java of the form:
    addVis("XXX",new VisualizerXXX(this),true);
This is added where all the other addVis() calls are in that code.
	
# Apps with sound
There are additional facilities for apps to access synthesizers or
control Ableton.  Let me know if this is of interest, and I will
provide some documentation.

# Running apps
Once Tracker is launched, it will open 2 windows; one showing the
display as it will be mapped to the ground, and another small GUI
control window.  The control window can be used to select which app is
currently active.  In the display window clicking and dragging will
add a simulated person with ID #1.  Typing a digit, 1-9, on the
keyboard will switch to controlling another person with that ID#.
Typing 'c' will clear all people.

# Questions/Comments/Help
Feel free to contact me with any questions or comments at `info@pulsefield.com` or check on the Pulsefield [website](http://www.pulsefield.com).
