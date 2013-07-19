class PulsefieldPS extends Pulsefield {
	HashMap<Integer, ParticleSystem> systems;
	PImage img;

	PulsefieldPS() {
		systems = new HashMap<Integer, ParticleSystem>();
		img = loadImage("texture.png");
	}

	synchronized void draw() {
		PGL pgl=((PGraphicsOpenGL)g).pgl;
		pgl.blendFunc(pgl.SRC_ALPHA, pgl.DST_ALPHA);
		pgl.blendEquation(pgl.FUNC_ADD);  
		background(0, 0, 0);  
		colorMode(RGB, 255);

		int toRemove=-1;
		for (Map.Entry me: systems.entrySet()) {
			ParticleSystem ps=(ParticleSystem)me.getValue();

			// Add gravitional attraction to all other systems
			for (Map.Entry m2: systems.entrySet()) {
				ParticleSystem ps2=(ParticleSystem)m2.getValue();
				// if (ps!=ps2) 
				//ps.attractor(ps2.origin, attractionForce);
				ps.push(ps2.origin, ps2.avgspeed);
			}

			ps.run();
			if (ps.enabled) {
				for (int k=0;k<birthrate;k++)
					ps.addParticle();
			}
			else if (ps.dead()) {
				println("ID "+me.getKey()+" is dead.");
				toRemove=(int)(Integer)me.getKey();
			}
		}
		if (toRemove!=-1) {
			systems.remove(toRemove);
			println("Removed system for ID "+toRemove);
		}
		if (systems.isEmpty()) {
			fill(50, 255, 255);
			textAlign(CENTER);
			textSize(32);
			text("Waiting for users...", width/2, height/2);
		}
	}


	synchronized void pfstopped() {
		super.pfstopped();
		systems.clear();
	}

	synchronized void pfupdate(int sampnum, float elapsed, int id, float ypos, float xpos, float yvelocity, float xvelocity, float majoraxis, float minoraxis, int groupid, int groupsize, int channel) {
		super.pfupdate(sampnum, elapsed, id, ypos, xpos, yvelocity, xvelocity, majoraxis, minoraxis, groupid, groupsize, channel);
		ParticleSystem ps=systems.get(id);
		if (ps==null) {
			println("Unable to locate particle system "+id+", creating it.");
			pfentry(sampnum, elapsed, id, channel);
			ps=systems.get(id);
		}

		ps.move(mapposition(xpos, ypos), elapsed);
		ps.enable(true);
	}

	synchronized void pfexit(int sampnum, float elapsed, int id) {
		super.pfexit(sampnum, elapsed, id);
		if (systems.containsKey(id)) {
			systems.get(id).enable(false);
		} 
		else
			println("Unable to locate particle system "+id);
	}

	synchronized void pfentry(int sampnum, float elapsed, int id, int channel) {
		super.pfentry(sampnum, elapsed, id, channel);
		color col=color((channel*37)%255, (channel*91)%255, (channel*211)%255);
		// col=color(255,255,255);
		println("Color="+col);
		systems.put(id, new ParticleSystem(0, mapposition(maxx, 0), col, img));
	}
}

