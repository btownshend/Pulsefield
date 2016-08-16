import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

class Nucleotide {
	private static final float RADIUS=0.05f;  
	private static final float DAMPING=0.2f; //0.2f;  // accel=-DAMPING * v
	private static final float SPINESTRENGTH=2f;
	static final float SPINESEP=0.15f;  // Mean separation between nucleotides on a strand
	static final float NOISE=0.05f;    // Random noise added to velocity
	static final float HBONDLENGTH=RADIUS*3;  // Mean length of hydrogen-bond between base pair
	static final float HBONDMAX=HBONDLENGTH*2;  // Breaking point of H-bond
	static final float HBONDSTRENGTH=0.1f;
	static final float OVERLAPFACTOR=1f;  // Strength of repulsion of overlapping nucleotides
	Nucleotide prior,next;   // 5' & 3' neighbors
	Nucleotide bp;    // Base-paired neighbor
	char label;
	PVector location;
	PVector velocity;
	
	Nucleotide(PVector pos, PVector vel, char label, Nucleotide prior) {
		location=new PVector(pos.x, pos.y);
		velocity=new PVector(vel.x, vel.y);
		this.label=label;
		this.prior=prior;
		this.next=null;
		if (prior!=null)
			prior.next=this;
		PApplet.println("Create nucleotide "+label+" at "+pos+" with velocity "+vel);
	}

	// Apply a force of given strength if separation from other is different from tgt
	void addBondForce(PVector other, float bondLength, float strength) {
		PVector sep=PVector.sub(other, location);
		float dist=sep.mag();
		PVector force=PVector.div(sep, sep.mag());
		if (dist>bondLength)
			force.mult((float)Math.pow(dist-bondLength,2f));
		else
			force.mult((float)-Math.pow(bondLength-dist,2));
		
		PVector acc=PVector.mult(force,  strength/Tracker.theTracker.frameRate);
		velocity.add(acc);
		if (acc.mag()>0.01f) 
			PApplet.println("Added "+acc+" to velocity of nucleotide at "+location);
	}
	
	// Compute effect of another nucleotide, n, on this one
	void interact(Nucleotide n) {
		assert(n!=this);
		if (prior==n || next==n) {
			// Adjacent nucleotide on same backbone
			addBondForce(n.location, SPINESEP, SPINESTRENGTH);
		} else {
			bpcheck(n);
			if (n==bp) {
				addBondForce(n.location, HBONDLENGTH, HBONDSTRENGTH);
			} 
		}
		
		PVector sep=PVector.sub(n.location, location);
		float dist=sep.mag();
		if (dist<RADIUS*2) {
			// Too close, separate them by exactly 2*RADIUS
			sep.normalize();  // Sep now points from this towards n
			PVector move=PVector.mult(sep, (RADIUS*2-dist)/2);
			location.sub(move);
			n.location.add(move);
			// And fuse their velocity components along the vector between them
			PVector v1=PVector.mult(sep, PVector.dot(velocity, sep));  // Velocity of this towards n
			PVector v2=PVector.mult(sep, PVector.dot(n.velocity, sep)); // Velocity of n away from this
			PVector avgVel=PVector.add(v1, v2).div(2f);
			velocity.sub(v1); velocity.add(PVector.div(avgVel,2f));
			n.velocity.sub(v2); velocity.add(PVector.div(avgVel,2));
			//PApplet.println("Overlap of "+location+" with "+n.location+": v1="+v1+", v2="+v2+", avg="+avgVel);
		}
	}
	
	// Check if this nucleotide will pair with another
	void bpcheck(Nucleotide n) {
		if (bp!=null || n.bp!=null)
			// Already in a basepair (update() takes care of breaking bonds)
			return;
		float dist=PVector.sub(n.location,location).mag();
		if (dist<HBONDMAX) {
			// Less than maximum length
			if ((label=='C'&&n.label=='G')||(label=='G'&&n.label=='C')||(label=='A'&&n.label=='T')||(label=='T'&&n.label=='A')) {	
				// Correct pairing
				if ((bp==null || dist<PVector.sub(bp.location, location).mag()) && (n.bp==null || dist<PVector.sub(n.bp.location, n.location).mag())) {
					if (bp!=null) {
						PApplet.println("Replaced base-pair between "+label+"@"+location+" and "+bp.label+"@"+bp.location+" with dist "+PVector.sub(bp.location, location).mag());
						bp.bp=null;
					}
					if (n.bp!=null) {
						PApplet.println("Replaced base-pair between "+n.label+"@"+location+" and "+n.bp.label+"@"+bp.location+" with dist "+PVector.sub(n.bp.location, n.location).mag());
						n.bp.bp=null;
					}
					bp=n;
					n.bp=this;
					PApplet.println("Formed new base-pair between "+label+"@"+location+" and "+n.label+"@"+n.location+" with dist "+dist);
				}
			}
		}
	}
	
	void update() {
		if (bp!=null) {
			float bpdist=PVector.sub(bp.location, location).mag();
			if (bpdist>HBONDMAX) {
				PApplet.println("Broke base-pair between "+label+"@"+location+" and "+bp.label+"@"+bp.location+" with dist "+PVector.sub(bp.location, location).mag());
				bp.bp=null;
				bp=null;
			}
		}
		// Damping
		velocity.add(PVector.mult(velocity, -DAMPING/Tracker.theTracker.frameRate));

		// Add random noise
		velocity.add(PVector.mult(PVector.random2D(),NOISE/Tracker.theTracker.frameRate));
		//PApplet.println(label+": v="+velocity);
		location.add(PVector.mult(velocity,1/Tracker.theTracker.frameRate));
		// Out of bounds, bounce back
		if (location.x+RADIUS > Tracker.maxx && velocity.x>0)
			velocity.x=-velocity.x;
		if (location.x-RADIUS < Tracker.minx && velocity.x<0)
			velocity.x=-velocity.x;
		if (location.y+RADIUS > Tracker.maxy && velocity.y>0)
			velocity.y=-velocity.y;
		if (location.y-RADIUS < Tracker.miny && velocity.y<0)
			velocity.y=-velocity.y;
	}
	
	public void draw(PGraphics g) {
//		PApplet.println("Draw nuc "+label+" at "+location);
		g.pushStyle();
		int color=0;
		switch (label) {
		case 'A':
			color=0xffff0000;
			break;
		case 'C':
			color=0xff00ff00;
			break;
		case 'G':
			color=0xff0000ff;
			break;
		case 'T':
			color=0xff7f7f00;
			break;
		}
		g.fill(color);
		g.stroke(color);
		g.strokeWeight(0.02f);
		g.ellipseMode(PConstants.CENTER);
		g.ellipse(location.x, location.y, RADIUS*2, RADIUS*2);
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		g.fill(255,255,255,255);  // White text
		Visualizer.drawText(g, RADIUS*1.5f, ""+label, location.x, location.y);
		if (bp!=null) {
			g.stroke(255,0,0,127);
			g.line(location.x, location.y, bp.location.x, bp.location.y);
		}
		g.popStyle();
	}
}
class Strand {
	PVector location;
	PVector velocity;
	float mass;
	boolean isAlive;
	private static HashSet<Strand> allStrands = new HashSet<Strand>();
	ArrayList<Nucleotide> nucs;   // Nucleotides in 5'->3' order
	
	Strand(float mass, PVector pos, PVector vel, String seq) {
		velocity=new PVector();
		velocity.x=vel.x; velocity.y=vel.y;
		location=new PVector();
		location.x=pos.x; location.y=pos.y;
		this.mass=mass;

		allStrands.add(this);
		//PApplet.println("Created Strand at "+location+" with velocity "+vel+" and mass "+mass);
		isAlive=true;
		nucs = new ArrayList<Nucleotide>();
		PVector spine=new PVector(Nucleotide.SPINESEP*(seq.length()-1),0);
		spine.rotate((float)(Math.random()*Math.PI*2));  // Random orientation
		Nucleotide prior=null;
		for (int i=0;i<seq.length();i++) {
			char base=seq.charAt(i);
			if (base=='N') {
				// Randomly choose a base
				final String bases="ACGT";
				base=bases.charAt((int)(Math.random()*4));
			}
			PVector nucpos=PVector.add(location, PVector.mult(spine, ((float)i)/(seq.length()-1)-0.5f));
			nucs.add(prior=new Nucleotide(nucpos,vel,base,prior));
		}
	}
	
	public void destroy() {
		//PApplet.println("Destroy Strand at "+location);
		allStrands.remove(this);
		isAlive=false;
	}
	public float getRadius() {
		// Return distance from center of mass to farthest nucleotide's center
		float r=0;
		for (Nucleotide nuc: nucs) {
			r=Math.max(r, PVector.sub(location,nuc.location).mag());
		}
		return r;
	}
	public PVector getMomentum() {
		return PVector.mult(velocity, mass);
	}
	public void update() {
		//location.x=0;location.y=0;
		velocity.x=0;velocity.y=0;
		for (Nucleotide nuc: nucs) {
			nuc.update();
			//location.add(nuc.location);
			velocity.add(nuc.velocity);
		}
		// Set location, velocity to average of nucleotides
		//location.div(nucs.size());
		velocity.div(nucs.size());
		// Actually, just use the 5' end as the location
		location=nucs.get(0).location;
	}
	public void draw(PGraphics g) {
		Nucleotide prev=null;
		g.pushStyle();
		g.stroke(255,127);
		g.strokeWeight(0.02f);
		for (Nucleotide nuc: nucs) {
			nuc.draw(g);
			if (prev!=null) {
				g.line(prev.location.x, prev.location.y, nuc.location.x, nuc.location.y);
			}
			prev=nuc;
		}
		g.popStyle();
	}
	static Strand create(float mass, PVector pos, PVector vel, String seq)  {
		return new Strand(mass,pos,vel,seq);
	}
	static void destroyAll() {
		Object all[]=allStrands.toArray();
		for (Object o: all) {
			Strand b=(Strand)o;
			b.destroy();
		}
	}
	
	static void updateAll(Effects effects) {
		Object all[]=allStrands.toArray();
		for (Object o: all) {
			Strand b=(Strand)o;
			b.update();
//			if (! (b  instanceof PlayerStrand)) {
//				// Player Strands destroyed by visualizer when person is lost
//				if (!Tracker.theTracker.inBounds(b.location))
//					// Out of bounds
//					b.destroy();
//			}
		}
		// Find all contacts
		for (Object o1: all) {
			Strand b1=(Strand)o1;
			if (!b1.isAlive)
				continue;
			for (Object o2: all) {
				Strand b2=(Strand)o2;
				if (!b2.isAlive)
					continue;
				for (Nucleotide n1:b1.nucs)
					for (Nucleotide n2:b2.nucs)
						if (n1!=n2)
							n1.interact(n2);
				
//				PVector sep=PVector.sub(b2.location, b1.location);
//				float minsep=b1.getRadius()+b2.getRadius();
//				if (sep.mag() <= minsep && b1.mass<b2.mass && b1.isAlive) {
//					// Contact
//					//PApplet.println("Contact at "+b1.location+" - "+b2.location);
//
//					float xfr=Math.min(b1.mass, MASSEXCHANGERATE/Tracker.theTracker.frameRate);
//					if (b2.mass<MAXMASS && !(b1 instanceof PlayerStrand && b1.mass-xfr<PlayerStrand.MINMASS)) {
//						b1.mass-=xfr;
//						b2.mass+=xfr;
//						b2.velocity=PVector.add(PVector.mult(b2.velocity,(b2.mass-xfr)/b2.mass), PVector.mult(b1.velocity, xfr/b2.mass));
//						//PApplet.println("Xfr "+xfr+", new masses: "+b1.mass+", "+b2.mass);
//						if (b1.mass==0)
//							b1.destroy();
//						else if (b1 instanceof PlayerStrand && b2 instanceof PlayerStrand) {
//							effects.play(((PlayerStrand)b2).person, "COLLIDE",127,1000);
//						}
//					}
//					minsep=b1.getRadius()+b2.getRadius(); // Update sep
//					float overlap=0.02f;   // Keep them overlapped up to this much
//					if (sep.mag() < minsep-overlap) {
//						// Keep them in exact contact (actually a little closer, so the stay in contact)
//						float movedist=minsep-overlap-sep.mag();
//						assert(movedist>0);
//						sep.normalize();
//						PVector m1=PVector.mult(sep,-movedist*b2.mass/(b1.mass+b2.mass));
//						PVector m2=PVector.mult(sep,movedist*b1.mass/(b1.mass+b2.mass));
//						//PApplet.println("Separate: sep="+sep+", minsep="+minsep+", movedist="+movedist,", m1="+m1+", m2="+m2);
//						b2.location.add(m2);
//						b1.location.add(m1);
//					}
					// Fuse their momentum
//					PVector momentum=PVector.add(b1.getMomentum(), b2.getMomentum());
//					b1.velocity=PVector.div(momentum, b1.mass+b2.mass);
//					b2.velocity=b1.velocity;
//				}
			}
		}
	}
	static void drawAll(PGraphics g) {
		for (Strand b: allStrands) {
			b.draw(g);
		}
	}
}

class PlayerStrand extends Strand {
	private static final float SPRINGCONSTANT=0.1f;   // force=SPRINGCONSTANT*dx  (Newtons/m)
	private static final float INITIALMASS=0.15f;  // In kg
	private static final String INITIALSEQ="NNNNNN";
	public static final float MINMASS=0.02f;    // In kg'
	PVector pilot;
	Person person;
	
	PlayerStrand(Person person, PVector pos, PVector vel) {
		super(INITIALMASS,pos,vel,INITIALSEQ);
		pilot=new PVector();
		pilot.x=pos.x;
		pilot.y=pos.y;
		this.person=person;
	}
	
	public void updatePosition(PVector newpilot) {
		pilot.x=newpilot.x; 
		pilot.y=newpilot.y;
	}
	
	@Override
	public void update() {
		for (Nucleotide nuc: nucs) {
			PVector delta=PVector.sub(pilot,nuc.location);
			PVector acc=PVector.mult(delta,SPRINGCONSTANT);  // Not really a spring, same accel indep of mass
			PVector deltaVelocity=PVector.mult(acc, nucs.size()/Tracker.theTracker.frameRate);
			nuc.velocity.add(deltaVelocity);
			break;  // Only pull the 5' end
		}
		super.update();
	}
	
	@Override
	public void draw(PGraphics g) {
		g.line(location.x, location.y, pilot.x, pilot.y);
		super.draw(g);
	}
}

public class VisualizerDNA extends VisualizerDot {
	long startTime;
	Effects effects;

	HashMap<Integer, PlayerStrand> strands;

	VisualizerDNA(PApplet parent, Synth synth) {
		super(parent);
		strands = new HashMap<Integer, PlayerStrand>();
		effects=new Effects(synth);
		effects.put("COLLIDE",new Integer[]{52,53,54,55});
		effects.put("SPLIT",new Integer[]{40,41,42});
	}
	
	public void update(PApplet parent, People allpos) {		
		// Update internal state of the Strands
		for (int id: allpos.pmap.keySet()) {
			if (!strands.containsKey(id))
				strands.put(id,new PlayerStrand(allpos.get(id),allpos.get(id).getOriginInMeters(),allpos.get(id).getVelocityInMeters()));
			PVector currentpos=allpos.get(id).getOriginInMeters();
			strands.get(id).updatePosition(currentpos);
			//PApplet.println("Strand "+id+" moved to "+currentpos.toString());
		}
		// Remove Strands for which we no longer have a position (exitted)
		for (Iterator<Integer> iter = strands.keySet().iterator();iter.hasNext();) {
			int id=iter.next().intValue();
			if (!allpos.pmap.containsKey(id)) {
				PApplet.println("Removing ID "+id);
				strands.get(id).destroy();
				iter.remove();
			}
		}
		Strand.updateAll(effects);
	}


	public void start() {
		super.start();
		strands = new HashMap<Integer, PlayerStrand>();
		Ableton.getInstance().setTrackSet("Osmos");
	}

	public void stop() {
		Strand.destroyAll();
		super.stop();
		PApplet.println("Stopping Osmos at "+System.currentTimeMillis());
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g,p);
		Strand.drawAll(g);
	}

	
}
