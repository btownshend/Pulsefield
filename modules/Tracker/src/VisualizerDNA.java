import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

import processing.core.PApplet;
import processing.core.PConstants;
import processing.core.PGraphics;
import processing.core.PVector;

class Nucleotide {
	// Distance constants (in meters)
	static final float RADIUS=0.05f;  
	static final float SPINESEP=RADIUS*3;  // Mean separation between nucleotides on a strand
	static final float LIGATEDIST=2*RADIUS;
	static final float SPINEBREAKDIST=1f;
	static final float HBONDLENGTH=RADIUS*3;  // Mean length of hydrogen-bond between base pair
	static final float HBONDMAX=HBONDLENGTH*2;  // Breaking length of H-bond

	static final float DAMPING=0.2f; //0.2f;  // accel=-DAMPING * v
	static final float DAMPVEL=0.5f;  // Damp only when velocity is >DAMPVEL
	static final float NOISE=0.05f;    // Random noise added to velocity (m/s)

	// Strengths of effects (i.e. acceleration=strength*force )
	static final float SPINESTRENGTH=2f;
	static final float HBONDSTRENGTH=2f;


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

	int strandLength() {
		if (next==null)
			return 1;
		else
			return 1+next.strandLength();
	}
	
	// Apply a force of given strength if separation from other is different from tgt
	void addBondForce(PVector other, float bondLength, float strength) {
		PVector sep=PVector.sub(other, location);
		float dist=sep.mag();
		PVector force=PVector.div(sep, dist);
		if (dist>bondLength)
			force.mult((float)Math.pow(dist-bondLength,2f));
		else
			force.mult((float)-Math.pow(bondLength-dist,2));
		
		PVector acc=PVector.mult(force,  strength/Tracker.theTracker.frameRate);
		velocity.add(acc);
//		if (acc.mag()>0.01f) 
//			PApplet.println("Added "+acc+" to velocity of nucleotide at "+location);
	}
	
	// Get the last nucleotide of this chain (the 3' one)
	Nucleotide getEnd() {
		if (next==null)
			return this;
		return next.getEnd();
	}
	
	int helixLength() {
		if (bp==null)
			return 0;
		int len=0;
		Nucleotide n=next;
		Nucleotide cn=bp.prior;
		while (n!=null && cn!=null && n.bp==cn) {
			len+=1;
			n=n.next;
			cn=cn.prior;
		}
		n=prior;
		cn=bp.next;
		while (n!=null && cn!=null & n.bp==cn) {
			len+=1;
			n=n.prior;
			cn=cn.next;
		}
		if (len>3)
			PApplet.println("Helix length ="+len+" at "+location);
		return len;
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
				addBondForce(n.location, HBONDLENGTH, HBONDSTRENGTH*(float)Math.pow(helixLength(),2));
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
			velocity.sub(v1); velocity.add(PVector.div(avgVel,1f));
			n.velocity.sub(v2); velocity.add(PVector.div(avgVel,1f));
			//PApplet.println("Overlap of "+location+" with "+n.location+": v1="+v1+", v2="+v2+", avg="+avgVel);
			if (next==null && n.prior==null) {
				// Check if they are already on the same strand (to avoid circularization)
				if (n.getEnd() == this)
					PApplet.println("Contact between strand ends at "+location+" and "+n.location);	
				else {
					// Ligate them
					PApplet.println("Ligating strands at "+location+" and "+n.location);
					next=n;
					n.prior=this;
				}
			}
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
						//PApplet.println("Replaced base-pair between "+label+"@"+location+" and "+bp.label+"@"+bp.location+" with dist "+PVector.sub(bp.location, location).mag());
						bp.bp=null;
					}
					if (n.bp!=null) {
						//PApplet.println("Replaced base-pair between "+n.label+"@"+location+" and "+n.bp.label+"@"+bp.location+" with dist "+PVector.sub(n.bp.location, n.location).mag());
						n.bp.bp=null;
					}
					bp=n;
					n.bp=this;
					//PApplet.println("Formed new base-pair between "+label+"@"+location+" and "+n.label+"@"+n.location+" with dist "+dist);
				}
			}
		}
	}
	
	void update(Effects e) {
		if (bp!=null) {
			float bpdist=PVector.sub(bp.location, location).mag();
			if (bpdist>HBONDMAX*helixLength()) {
				//PApplet.println("Broke base-pair between "+label+"@"+location+" and "+bp.label+"@"+bp.location+" with dist "+PVector.sub(bp.location, location).mag());
				bp.bp=null;
				bp=null;
			}
			// Check if hydrogen bonds are mediating a splinted ligation
			if (bp!=null && prior==null && bp.next!=null && bp.next.bp!=null && bp.next.bp.next==null) {
				PApplet.println("Splinted Ligation");
				prior=bp.next.bp;
				prior.next=this;
			}
		}
		// Damping
		if (velocity.mag()>DAMPVEL)
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
		
		// Check for breakage
		if (next!=null) {
			float sep=PVector.sub(location, next.location).mag();
			if (sep>SPINEBREAKDIST) {
				PApplet.println("Breaking spine between "+location+" and "+next.location);
				next.prior=null;
				new Strand(next);
				next=null;
				e.play("BREAK", 127, 1000);
			}
		}
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
		float drawRadius=RADIUS;
		if (prior==null) {
			g.stroke(0xff7f007f);  // 5' end
			drawRadius*=1.5f;  // Bigger
		} else if (next==null)
			g.stroke(0xff007f7f);  // 3' end
		else
			g.stroke(color);
		g.strokeWeight(0.02f);
		g.ellipseMode(PConstants.CENTER);
		g.rectMode(PConstants.CENTER);
		if (prior==null)
			g.rect(location.x, location.y, drawRadius*2, drawRadius*2);
		else
			g.ellipse(location.x, location.y, drawRadius*2, drawRadius*2);
		g.textAlign(PConstants.CENTER,PConstants.CENTER);
		g.fill(255,255,255,255);  // White text
		Visualizer.drawText(g, RADIUS*1.8f, ""+label, location.x, location.y-drawRadius*0.2f);
		if (bp!=null) {
			g.stroke(255,0,0,127);
			g.line(location.x, location.y, bp.location.x, bp.location.y);
		}
		g.popStyle();
	}
}
class Strand {
	PVector velocity;
	boolean isAlive;
	private static HashSet<Strand> allStrands = new HashSet<Strand>();
	Nucleotide nucs;   // Nucleotides in 5'->3' order
	
	Strand(PVector pos, PVector vel, String seq) {
		allStrands.add(this);
		//PApplet.println("Created Strand at "+location+" with velocity "+vel+" and mass "+mass);
		isAlive=true;
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
			PVector nucpos=PVector.add(pos, PVector.mult(spine, ((float)i)/(seq.length()-1)-0.5f));
			Nucleotide nn=new Nucleotide(nucpos,vel,base,prior);
			if (i==0)
				nucs=nn;
			prior=nn;
		}
	}
	
	// Make an already existing nucleotide chain into a new strand
	Strand(Nucleotide n) {
		allStrands.add(this);
		isAlive=true;
		assert(n.prior==null);
		nucs=n;
	}
	
	public void destroy() {
		//PApplet.println("Destroy Strand at "+location);
		allStrands.remove(this);
		isAlive=false;
	}

	public PVector location() {
		return nucs.location;
	}
	
	public void update(Effects e) {
		// Check if this strand has been ligated onto the end of another one
		if (nucs.prior != null) {
			if (this instanceof PlayerStrand) {
				PApplet.println("Found a player strand pointing to a ligated chain, breaking it");
				nucs.prior.next=null;
				nucs.prior=null;
			} else {
				PApplet.println("Found a strand pointing to a ligated chain, destroying it");
				e.play("LIGATE", 127, 1000);
				destroy();
				return;
			}
		}
		if (nucs.prior!=null)
			// No update, the strand
			return;
		
		for (Nucleotide nuc=nucs;nuc!=null;nuc=nuc.next) {
			nuc.update(e);
		}
	}
	public void draw(PGraphics g) {
		Nucleotide prev=null;
		g.pushStyle();
		g.stroke(255,127);
		g.strokeWeight(0.02f);
		for (Nucleotide nuc=nucs;nuc!=null;nuc=nuc.next) {
			nuc.draw(g);
			if (prev!=null) {
				PVector indent=PVector.sub(prev.location, nuc.location);
				indent.normalize();
				indent.mult(Nucleotide.RADIUS);
				PVector p1=PVector.sub(prev.location, indent);
				PVector p2=PVector.add(nuc.location, indent);
				g.line(p1.x,p1.y,p2.x,p2.y);
			}
			prev=nuc;
		}
		g.popStyle();
	}
	static Strand create(PVector pos, PVector vel, String seq)  {
		return new Strand(pos,vel,seq);
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
			b.update(effects);
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
				for (Nucleotide n1=b1.nucs;n1!=null;n1=n1.next)
					for (Nucleotide n2=b2.nucs;n2!=null;n2=n2.next)
						if (n1!=n2)
							n1.interact(n2);
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
	private static final float SPRINGCONSTANT=0.5f;   // force=SPRINGCONSTANT*dx  (Newtons/m)
	private static final String INITIALSEQ="ACCGGATCCGGT";
	PVector pilot;
	Person person;
	
	PlayerStrand(Person person, PVector pos, PVector vel) {
		super(pos,vel,INITIALSEQ);
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
	public void update(Effects e) {
		PVector delta=PVector.sub(pilot,location());
		PVector acc=PVector.mult(delta,SPRINGCONSTANT);  // Not really a spring, same accel indep of mass
		PVector deltaVelocity=PVector.mult(acc, 1/Tracker.theTracker.frameRate);
		nucs.velocity.add(deltaVelocity);
		super.update(e);
	}
	
	@Override
	public void draw(PGraphics g) {
		g.line(location().x, location().y, pilot.x, pilot.y);
		super.draw(g);
	}
}

public class VisualizerDNA extends VisualizerDot {
	static final int NUMRANDSTRANDS=10;
	long startTime;
	Effects effects;

	HashMap<Integer, PlayerStrand> strands;

	VisualizerDNA(PApplet parent) {
		super(parent);
		strands = new HashMap<Integer, PlayerStrand>();
		this.effects=Effects.defaultEffects;
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
		Ableton.getInstance().setTrackSet("DNA");
		for (int i=0;i<NUMRANDSTRANDS;i++)
			new Strand(new PVector((float)Math.random()*(Tracker.maxx-Tracker.minx)+Tracker.minx,(float)Math.random()*(Tracker.maxy-Tracker.miny)+Tracker.miny),
					PVector.random2D(),"NNNN");
	}

	public void stop() {
		Strand.destroyAll();
		super.stop();
		PApplet.println("Stopping DNA at "+System.currentTimeMillis());
	}

	@Override
	public void draw(Tracker t, PGraphics g, People p) {
		super.draw(t, g,p);
		Strand.drawAll(g);
	}

	
}
