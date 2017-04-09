package com.pulsefield.tracker;
import java.awt.Color;

import codeanticode.syphon.SyphonClient;
import oscP5.OscMessage;
import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PImage;
import processing.core.PVector;

class NavierOFSettings {
	int index;  // Which setting# this is
	float viscosity, diffusion, dissipation, velocityDissipation, tempDissipation, pressDissipation, limitVelocity; //  Parameters of model
	float ambient;
	float smokeBuoyancy, smokeWeight;
	boolean smokeEnable;
	int iterations;
	float alpha, legScale, velScale, saturation, brightness, density, temperature;  // Parameters of leg effects
	float flameTemperature, flameDensity, flameRadius;
	boolean flameEnable;
	boolean multiColor, rainbow, border;
	PVector flamePosition, flameVelocity, gravity;
	boolean modified;  // True of any settings changed, but not written
	
	NavierOFSettings(int _index) {
		index=_index;
		loadPreset();
	}
	
	public void loadPreset() {
		String group="navier"+index;
		ambient=Config.getFloat(group,"ambient",0f);
		smokeBuoyancy=Config.getFloat(group,"smokeBuoyancy",1.0f);
		smokeWeight=Config.getFloat(group,"smokeWeight",0.05f);
		smokeEnable=Config.getInt(group,"smokeEnable",1)!=0;;
		diffusion=Config.getFloat(group,"diffusion",0.1f);
		dissipation=Config.getFloat(group,"dissipation",0.99f);
		velocityDissipation=Config.getFloat(group,"velocityDissipation",0.99f);
		pressDissipation=Config.getFloat(group,"pressDissipation",0.9f);
		tempDissipation=Config.getFloat(group,"tempDissipation",0.99f);
		gravity=Config.getVec(group,"gravity",new PVector(0f,-0.98f));
		viscosity=Config.getFloat(group,"viscosity",0.1f);
		alpha=Config.getFloat(group,"alpha",1.0f);
		legScale=Config.getFloat(group,"legScale",1.0f);
		velScale=Config.getFloat(group,"velScale",20.0f);
		saturation=Config.getFloat(group,"saturation",1.0f);
		brightness=Config.getFloat(group,"brightness",1.0f);
		temperature=Config.getFloat(group,"temperature",10f);
		density=Config.getFloat(group,"density",1f);
		limitVelocity = Config.getFloat(group,"limitVelocity",200f);
		flamePosition = Config.getVec(group,"flamePosition",new PVector(0,0.7f));
		flameVelocity = Config.getVec(group,"flameVelocity",new PVector(0,-2));
		flameRadius = Config.getFloat(group,"flameRadius",10f);
		flameTemperature = Config.getFloat(group,"flameTemperature",10f);
		flameDensity = Config.getFloat(group,"flameDensity",1f);
		flameEnable = Config.getInt(group,"flameEnable",1)!=0;
		iterations = Config.getInt(group,"iterations",40);
		multiColor = Config.getInt(group,"multiColor",1)!=0;
		rainbow = Config.getInt(group,"rainbow",1)!=0;
		border = Config.getInt(group,"border",1)!=0;
		modified = false;
	}
	
	public void savePreset() {
		String group="navier"+index;

		PApplet.println("Saving presets for Navier "+index);
		Config.setFloat(group,"ambient",ambient);
		Config.setFloat(group,"smokeBuoyancy",smokeBuoyancy);
		Config.setFloat(group,"smokeWeight",smokeWeight);
		Config.setInt(group,"smokeEnable",smokeEnable?1:0);
		Config.setFloat(group,"diffusion",diffusion);
		Config.setFloat(group,"dissipation",dissipation);
		Config.setFloat(group,"velocityDissipation",velocityDissipation);
		Config.setFloat(group,"pressDissipation",pressDissipation);
		Config.setFloat(group,"tempDissipation",tempDissipation);
		Config.setVec(group,"gravity",gravity);
		Config.setFloat(group,"viscosity",viscosity);
		Config.setFloat(group,"alpha",alpha);
		Config.setFloat(group,"legScale",legScale);
		Config.setFloat(group,"velScale",velScale);
		Config.setFloat(group,"saturation",saturation);
		Config.setFloat(group,"brightness",brightness);
		Config.setFloat(group,"temperature",temperature);
		Config.setFloat(group,"density",density);
		Config.setFloat(group,"limitVelocity",limitVelocity);
		Config.setVec(group,"flamePosition",flamePosition);
		Config.setVec(group,"flameVelocity",flameVelocity);
		Config.setFloat(group,"flameRadius",flameRadius);
		Config.setFloat(group,"flameTemperature",flameTemperature);
		Config.setFloat(group,"flameDensity",flameDensity);
		Config.setInt(group,"flameEnable",flameEnable?1:0);
		Config.setInt(group,"iterations",iterations);
		Config.setInt(group,"multiColor",multiColor?1:0);
		Config.setInt(group,"rainbow",rainbow?1:0);
		Config.setInt(group,"border",border?1:0);
		modified=false;
	}
	
	public void updateOF() {
		;
	}

	public void setOF(String name, double value) {
		// Send to OF
		OscMessage set = new OscMessage("/navier/"+name);
		set.add((float)value);
		OFOSC.getInstance().sendMessage(set);
	}

	public void setOF(String name, int value) {
		// Send to OF
		OscMessage set = new OscMessage("/navier/"+name);
		set.add(value);
		OFOSC.getInstance().sendMessage(set);
	}

	public void setOF(String name, String value) {
		// Send to OF
		OscMessage set = new OscMessage("/navier/"+name);
		set.add(value);
		OFOSC.getInstance().sendMessage(set);
	}
	
	public void setOF(String name, double v1, double v2) {
		// Send to OF
		OscMessage set = new OscMessage("/navier/"+name);
		set.add((float)v1);
		set.add((float)v2);
		OFOSC.getInstance().sendMessage(set);
	}

	public void handleMessage(OscMessage msg) {
		//PApplet.println("Navier message: "+msg.toString());
		String pattern=msg.addrPattern();
		String components[]=pattern.split("/");

		if (components.length==4 && components[3].equals("dissipation")) {
			dissipation=(float) (1-Math.pow(10f,msg.get(0).floatValue()));
		} else if (components.length==4 && components[3].equals("velocityDissipation")) {
			velocityDissipation=(float) (1-Math.pow(10f,msg.get(0).floatValue()));
		} else if (components.length==4 && components[3].equals("tempDissipation")) {
			tempDissipation=(float) (1-Math.pow(10f,msg.get(0).floatValue()));
		} else if (components.length==4 && components[3].equals("pressDissipation")) {
			pressDissipation=(float) (1-Math.pow(10f,msg.get(0).floatValue()));
		} else if (components.length==4 && components[3].equals("iterations")) {
			iterations=(int)msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("viscosity")) {
			viscosity=(float) (Math.pow(10f,msg.get(0).floatValue()));
		} else if (components.length==4 && components[3].equals("diffusion")) {
			diffusion=(float) (Math.pow(10f,msg.get(0).floatValue()));
		} else if (components.length==4 && components[3].equals("ambient")) {
			ambient=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("saturation")) {
			saturation=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("brightness")) {
			brightness=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("alpha")) {
			alpha=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("legscale")) {
			legScale=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("velscale")) {
			velScale=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("temperature")) {
			temperature=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("density")) {
			density=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("multicolor")) {
			multiColor=msg.get(0).floatValue()>0.5f;
		} else if (components.length==4 && components[3].equals("rainbow")) {
			rainbow=msg.get(0).floatValue()>0.5f;
		} else if (components.length==4 && components[3].equals("border")) {
			border=msg.get(0).floatValue()>0.5f;
		} else if (components.length==4 && components[3].equals("gravityClear") ) {
			gravity.x=0f; gravity.y=0f;
		} else if (components.length==4 && components[3].equals("gravity") ) {
			gravity.y=msg.get(0).floatValue();
			gravity.x=msg.get(1).floatValue();
		} else if (components.length==4 && components[3].equals("flameTemperature")) {
			flameTemperature=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("flameDensity")) {
			flameDensity=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("flameRadius") ) {
			flameRadius=msg.get(0).floatValue();
		} else if (components.length==4 && components[3].equals("flameEnable") ) {
			flameEnable=msg.get(0).floatValue()>0.5;
		} else if (components.length==4 && components[3].equals("flamePosition") ) {
			flamePosition.y=msg.get(0).floatValue();
			flamePosition.x=msg.get(1).floatValue();
		} else if (components.length==4 && components[3].equals("flameVelocity") ) {
			flameVelocity.y=msg.get(0).floatValue();
			flameVelocity.x=msg.get(1).floatValue();
		} else if (components.length==5 && components[3].equals("smoke")) {
			if (components[4].equals("buoyancy"))
				smokeBuoyancy=msg.get(0).floatValue();
			else if (components[4].equals("weight"))
				smokeWeight=msg.get(0).floatValue();
			else if (components[4].equals("enable"))
				smokeEnable=msg.get(0).floatValue()>0.5f;
			else
				PApplet.println("Unknown NavierOF Message: "+msg.toString());
		} else if (components.length==4 && components[3].equals("savepreset") ) {
			if (msg.get(0).floatValue() > 0.5f)
				savePreset();
			return;
		} else if (components.length==4 && components[3].equals("loadpreset") ) {
			if (msg.get(0).floatValue() > 0.5f)
				loadPreset();
			return;
		} else {
			PApplet.println("Unknown NavierOF Message: "+msg.toString());
			return;
		}
		PApplet.println("dissipation="+dissipation+", velocityDissipation="+velocityDissipation+", gravity="+gravity);
		PApplet.println("flame at "+flamePosition+", vel="+flameVelocity+"enable="+flameEnable+", temp="+flameTemperature+", radius="+flameRadius+",den="+flameDensity);
		modified=true;
		setTO();
	}
	
	void setTOValue(String name, double value, String fmt) {
		TouchOSC to=TouchOSC.getInstance();
		OscMessage set = new OscMessage("/video/navierOF/"+name);
		set.add(value);
		to.sendMessage(set);
		set=new OscMessage("/video/navierOF/"+name+"/value");
		set.add(String.format(fmt, value));
		to.sendMessage(set);
	}
	void setTOValue(String name, double v1, double v2, String fmt) {
		TouchOSC to=TouchOSC.getInstance();
		OscMessage set = new OscMessage("/video/navierOF/"+name);
		set.add(v1);
		set.add(v2);
		to.sendMessage(set);
		set=new OscMessage("/video/navierOF/"+name+"/value");
		set.add(String.format(fmt, v1, v2));
		to.sendMessage(set);
	}

	public void setTO() {
		setTOValue("ambient",ambient,"%.2f");
		setTOValue("iterations",iterations,"%.0f");
		setTOValue("viscosity",Math.log10(viscosity),"%.2f");
		setTOValue("diffusion",Math.log10(diffusion),"%.2f");
		setTOValue("dissipation",Math.log10(1-dissipation),"%.2f");
		setTOValue("velocityDissipation",Math.log10(1-velocityDissipation),"%.2f");
		setTOValue("tempDissipation",Math.log10(1-tempDissipation),"%.2f");
		setTOValue("pressDissipation",Math.log10(1-pressDissipation),"%.2f");
		setTOValue("gravity",gravity.y, gravity.x,"%.2f,%.2f");
		setTOValue("brightness",brightness,"%.2f");
		setTOValue("saturation",saturation,"%.2f");
		setTOValue("alpha",alpha,"%.2f");
		setTOValue("velscale",velScale,"%.2f");
		setTOValue("legscale",legScale,"%.2f");
		setTOValue("temperature",temperature,"%.2f");
		setTOValue("density",density,"%.2f");
		setTOValue("multicolor",multiColor?1.0f:0.0f,"%.0f");
		setTOValue("rainbow",rainbow?1.0f:0.0f,"%.0f");
		setTOValue("border",border?1.0f:0.0f,"%.0f");
		setTOValue("flameTemperature",flameTemperature,"%.2f");
		setTOValue("flameDensity",flameDensity,"%.2f");
		setTOValue("flameRadius",flameRadius,"%.2f");
		setTOValue("flameEnable",flameEnable?1.0:0.0,"%.0f");
		setTOValue("flamePosition",flamePosition.y,flamePosition.x,"%.2f,%.2f");
		setTOValue("flameVelocity",flameVelocity.y,flameVelocity.x,"%.2f,%.2f");
		setTOValue("smoke/buoyancy",smokeBuoyancy,"%.2f");
		setTOValue("smoke/weight",smokeWeight,"%.2f");
		setTOValue("smoke/enable",smokeEnable?1.0:0.0,"%.0f");
		setTOValue("preset/"+Integer.toString(8-index)+"/1",1.0f,"%.0f");
		setTOValue("presetled/"+Integer.toString(index),modified?1.0f:0.0f,"%.0f");
		setOF("ambient",ambient);
		setOF("viscosity",viscosity);
		setOF("diffusion",diffusion);
		setOF("iterations",iterations);
		setOF("dissipation",dissipation);
		setOF("velocityDissipation",velocityDissipation);
		setOF("temperatureDissipation",tempDissipation);
		setOF("pressureDissipation",pressDissipation);
		setOF("gravity",gravity.x,gravity.y);
		// Send flame settings to OF
		OscMessage set = new OscMessage("/navier/flame");
		set.add(flameEnable?1.0f:0.0f);
		set.add(flamePosition.x);
		set.add(flamePosition.y);
		set.add(flameVelocity.x);
		set.add(flameVelocity.y);
		set.add(flameDensity);
		set.add(flameTemperature);
		set.add(flameRadius);
		OFOSC.getInstance().sendMessage(set);
		
		// Send smoke settings
		set = new OscMessage("/navier/smoke");
		set.add(smokeEnable?1.0f:0.0f);
		set.add(smokeBuoyancy);
		set.add(smokeWeight);
		OFOSC.getInstance().sendMessage(set);
	}

	}

	class VisualizerNavierOF extends VisualizerSyphon {
		NavierOFSettings[] settings;
		SyphonClient clients[];
		int syphonTexture;  // Which texture to display 0:normal,1:vel,2:temp,3:press

		float canvasScale=0.5f;   // Relative size of OF canvas to local floor image
		int currentSettings;
		private int bordercolor;
		PImage buffer=null;
		PImage iao;
		int rainbow = 0;
		long statsLast = 0;
		long statsTick = 0;
		long statsStep = 0;
		long statsUpdate = 0;
		long statsU1=0;
		long statsU2=0;
		long statsU3=0;
		long statsU4=0;
		Synth synth;
		//MusicVisLaser mvl;
		final int downSample=2;   // amount to downsample fluid image

		VisualizerNavierOF(Tracker parent, Synth synth, String appName) {
			super(parent, appName, "Main");

			// iao = loadImage("IAO.jpg");
			//background(iao);
			settings=new NavierOFSettings[8];
			for (int i=0;i<settings.length;i++) {
				settings[i]=new NavierOFSettings(i);
			}
			currentSettings=0;
			settings[currentSettings].setTO();
			stats();

			this.synth=synth;
			//mvl=new MusicVisLaser(parent.fourier, MusicVisLaser.Modes.POLYGON);
		}

		@Override
		public void start() {
			super.start();
			// Client contains main window (density), setup array of additional clients
			clients=new SyphonClient[4];
			clients[0]=client;
			clients[1]=initClient(appName,"Velocity");
			clients[2]=initClient(appName,"Temperature");
			clients[3]=initClient(appName,"Pressure");
			syphonTexture = 0;
			Laser.getInstance().setFlag("body",0.0f);
			Laser.getInstance().setFlag("legs",0.0f);
			Ableton.getInstance().setTrackSet("Navier");
		}

		@Override
		public void stop() {
			super.stop();  // Stops clients[0]
			clients[0]=null;
			for (int i=1;i<clients.length;i++) {
				clients[i].stop();
				clients[i]=null;
			}
		}

		public void handleMessage(OscMessage msg) {
			PApplet.println("Navier message: "+msg.toString());
			String pattern=msg.addrPattern();
			String components[]=pattern.split("/");

			if (components.length<3 || !components[2].equals("navierOF")) {
				PApplet.println("Navier: Expected /video/navierOF messages, got "+msg.toString());
			} else if (components.length==4 && components[3].equals("capture")  && msg.get(0).floatValue()>0.5f) {
				settings[currentSettings].setOF("capture","/tmp/ofcapt");  // Capture a snapshot on both sides of syphon
				captureNextFrame=true;
			} else if (components.length==6 && components[3].equals("preset") ) {
				int newSetting=8-Integer.valueOf(components[4]);
				if (newSetting>=0 && newSetting <settings.length && msg.get(0).floatValue()>0.5f)
					currentSettings=newSetting;
				PApplet.println("using settings "+currentSettings);
			} else if (components.length==4 && components[3].equals("quit") ) {
				settings[currentSettings].setOF("quit",1);  // Make OF implementation exit
			} else if (components.length==4 && components[3].equals("clear") ) {
				settings[currentSettings].setOF("clear",1);  // Clear frame buffers
			} else if (components.length==4 && components[3].equals("frozen") ) {
				settings[currentSettings].setOF("frozen",msg.get(0).floatValue());  // Freeze
			} else if (components.length==6 && components[3].equals("texselect") ) {
				if (msg.get(0).floatValue()>0.5f)
					syphonTexture=Integer.valueOf(components[5])-1;
				PApplet.println("syphon texture="+syphonTexture);
			} else if (currentSettings>0)  // Not defaults
				settings[currentSettings].handleMessage(msg);
			settings[currentSettings].setTO();
			settings[currentSettings].setTOValue("texselect/1/"+Integer.toString(syphonTexture+1),1.0f,"%.0f");
		}


		public void stats() {
			long elapsed=System.nanoTime()-statsLast;
			PApplet.println("Total="+elapsed/1e6+"msec , Tick="+statsTick*100f/elapsed+"%, Step="+statsStep*100f/elapsed+"%, Update="+statsUpdate*100f/elapsed+"%");
			PApplet.println("U=",statsU1*100f/elapsed,", ",statsU2*100f/elapsed,", ",statsU3*100f/elapsed,", ",statsU4*100f/elapsed);
			statsLast = System.nanoTime();
			statsTick=0;
			statsStep=0;
			statsUpdate=0;
			statsU1=0;
			statsU2=0;
			statsU3=0;
			statsU4=0;
		}

		void applyForce(int cellX, int cellY, double dx, double dy, float red, float green, float blue, float alpha, float radius, float temp, float dens) {
			OscMessage msg = new OscMessage("/navier/force");
			msg.add(cellX);
			msg.add(cellY);
			msg.add((float)dx);
			msg.add((float)dy);
			msg.add(red/255.0f);
			msg.add(green/255.0f);
			msg.add(blue/255.0f);
			msg.add(alpha);
			msg.add(radius);
			msg.add(temp);
			msg.add(dens);
			//PApplet.println("red="+(red/255.0)+", green="+(green/255.0)+", blue="+(blue/255.0));
			OFOSC.getInstance().sendMessage(msg);
		}

		void setBorder(boolean onOff, float red, float green, float blue) {
			OscMessage msg = new OscMessage("/navier/border");
			msg.add(onOff?1.0f:0.0f);
			msg.add(red/255.0f);
			msg.add(green/255.0f);
			msg.add(blue/255.0f);

			//PApplet.println("red="+(red/255.0)+", green="+(green/255.0)+", blue="+(blue/255.0));
			OFOSC.getInstance().sendMessage(msg);
		}
		void updateForces() {
			OscMessage msg = new OscMessage("/navier/updateForces");
			OFOSC.getInstance().sendMessage(msg);
		}

		@Override
		public void update(PApplet parent, People p) {
			Ableton.getInstance().updateMacros(p);
			// Make sure VisualizerSyphon client points to correct syphon client
			client=clients[syphonTexture];
			assert(client!=null);
			for (Person pos: p.pmap.values()) {
				//PApplet.println("ID "+pos.id+" avgspeed="+pos.avgspeed.mag());
				if (pos.isMoving())
					synth.play(pos.id,pos.channel+35,127,480,pos.channel);
			}
			long t1=System.nanoTime();
			int nwidth=1024, nheight=1024;
			if (canvas!=null) {
				nwidth=canvas.width;
				nheight=canvas.height;
			}
			for (Person pos: p.pmap.values()) {
				//PApplet.println("update("+p.channel+"), enabled="+p.enabled);
				for (int l=0;l<pos.legs.length;l++) {
					Leg leg = pos.legs[l];
					int cellX = (int)( (-leg.getNormalizedPosition().x+1)*nwidth / 2);
					cellX=Math.max(0,Math.min(cellX,nwidth-1));
					int cellY = (int) ((leg.getNormalizedPosition().y+1)*nheight/ 2);
					cellY=Math.max(0,Math.min(cellY,nheight-1));

					double dx=leg.getNormalizedVelocity().x*nwidth/2; // In pixels/sec
					double dy=leg.getNormalizedVelocity().y*nheight/2;
					float radius=leg.getDiameterInMeters()/Tracker.getFloorSize().x*nwidth/2*settings[currentSettings].legScale;
					int c;
					if (settings[currentSettings].multiColor) {
						//  c=parent.color((int)(128+127*Math.sin(parent.frameCount/101f)),(int)(128+127*Math.sin(parent.frameCount/93f)),(int)(128+127*Math.sin(parent.frameCount/107f)));
						float colorPeriod=20;   // Cycle through all colors in this many seconds
						float hue=(float)(pos.id*110+l*20+128*Math.sin(2*Math.PI*parent.frameCount/30/colorPeriod))/255f;
//						if (l==0 && parent.frameCount%10==0)
//							PApplet.println("frame="+parent.frameCount+", id="+pos.id+", hue="+hue+", cell="+cellX+","+cellY);
						c=Color.HSBtoRGB(hue,settings[currentSettings].saturation,settings[currentSettings].brightness);
					} else
						// No change
						c=Color.HSBtoRGB(((pos.id*110+l*20)&0xff)/255.0f,settings[currentSettings].saturation,settings[currentSettings].brightness);
				
					//PApplet.println("Leg "+l+": Cell="+cellX+","+cellY+", vel="+dx+","+dy+ ", radius="+radius+", color="+PApplet.hex(c));
					dx = (Math.abs(dx) > settings[currentSettings].limitVelocity) ? Math.signum(dx) * settings[currentSettings].limitVelocity : dx;
					dy = (Math.abs(dy) > settings[currentSettings].limitVelocity) ? Math.signum(dy) * settings[currentSettings].limitVelocity : dy;
					dx *= settings[currentSettings].velScale;
					dy *= settings[currentSettings].velScale;
					applyForce(cellX, cellY, dx, dy, parent.red(c), parent.green(c),parent.blue(c),settings[currentSettings].alpha,radius,settings[currentSettings].temperature,settings[currentSettings].density);
				}
			}

			int c;
			if (settings[currentSettings].rainbow) {
				float colorPeriod=20;   // Cycle through all colors in this many seconds
				float hue=(float)(0.5+0.5*Math.sin(2*Math.PI*parent.frameCount/30/colorPeriod))/255f;
				c=Color.HSBtoRGB(hue,settings[currentSettings].saturation,settings[currentSettings].brightness);
			} else 
				c=0;
			setBorder(settings[currentSettings].border,parent.red(c), parent.green(c),parent.blue(c));

			if (p.pmap.isEmpty()) {
				// Keep it moving
				//applyForce((int)(Math.random()*nwidth), (int)(Math.random()*nheight), limitVelocity/8*(Math.random()*2-1), limitVelocity/8*(Math.random()*2-1));
			}
			updateForces();
			statsUpdate += System.nanoTime()-t1;
			//stats();
		}

		public int lerpColor(PApplet parent, int c1, int c2, float l) {
			parent.colorMode(PConstants.RGB, 255);
			float r1 = parent.red(c1)+0.5f;
			float g1 = parent.green(c1)+0.5f;
			float b1 = parent.blue(c1)+0.5f;

			float r2 = parent.red(c2)+0.5f;
			float g2 = parent.green(c2)+0.5f;
			float b2 = parent.blue(c2)+0.5f;

			return parent.color( PApplet.lerp(r1, r2, l), PApplet.lerp(g1, g2, l), PApplet.lerp(b1, b2, l) );
		}
		@Override
		public void draw(Tracker t, PGraphics g, People p) {
			super.draw(t, g, p);
			if (canvas!=null) {
				int newWidth=(int)(g.width*canvasScale);
				int newHeight=(int)(g.height*canvasScale);
				if (canvas.width!=newWidth || canvas.height!=newHeight) {
					PApplet.println("Resetting OF size from "+canvas.width+","+canvas.height+" to "+newWidth+","+newHeight);
					OscMessage set = new OscMessage("/navier/setsize");
					set.add(newWidth);
					set.add(newHeight);
					set.add(1.0f);  // Just display at same resolution as internal textures
					OFOSC.getInstance().sendMessage(set);
				}
			}
		}
	}

