#include <algorithm>
#include <numeric>
#include "lo/lo.h"
#include "world.h"
#include "vis.h"
#include "dbg.h"
#include "parameters.h"

static const bool REASSIGNPOINTS=false;

World::World(float maxRange): groups(GROUPDIST,UNGROUPDIST) {
    lastframe=0;
    priorngroups=0;
    initWindow();
    drawRange=true;
    miny=0;
    maxy=maxRange;
    minx=-maxRange;
    maxx=maxRange;
}

// Class for managing targets based on lidar scan before assigning them to people
class Target {
    std::vector<int> scans;
    std::vector<Point> hits;
    bool assigned;
public:
    Target() { assigned=false; }

    // Get maximum distance to given point
    float getDist(Point p) const { 
	    float tgtdist=0;
	    for (int i=0;i<hits.size();i++)
		tgtdist=std::max(tgtdist, (p-hits[i]).norm());
	    return tgtdist;
    }
    void append(int scan,Point pos) { scans.push_back(scan); hits.push_back(pos); }
    Point lastHit() const { return hits.back(); }
    int lastScan() const { return scans.back(); }
    // Get estimate of center of target
    Point getCenter() const {
	Point sum(0,0);
	for (int i=0;i<hits.size();i++)
	    sum=sum+hits[i];
	Point mid=sum/hits.size();  // Mean of points
	float offset=INITLEGDIAM/2*(M_PI/4);   // Shift away from sensor by mean dist one would be in front of a circle of given diameter
	Point center=mid*(1+offset/mid.norm());
	return center;
    }
    float getWidth() const { return (hits.front()-hits.back()).norm(); }
    // Split target at largest jump
    Target split() {
	assert(hits.size()>=2);
	int jump=0;   // Jump is after this index
	float jumpSize=(hits[jump]-hits[jump+1]).norm();
	for (int i=1;i<hits.size()-1;i++) {
	    float jtmp=(hits[i]-hits[i+1]).norm();
	    if (jtmp>jumpSize) {
		jump=i;
		jumpSize=jtmp;
	    }
	}
	dbg("World.makeAssignments",3) << "Split target into 2 at " << scans[0] << "-" << scans[jump] << " and " << scans[jump+1] << "-" << scans.back() << std::endl;
	Target result;
	for (int i=jump+1;i<hits.size();i++)
	    result.append(scans[i],hits[i]);
	hits=std::vector<Point>(hits.begin(),hits.begin()+jump+1);
	scans=std::vector<int>(scans.begin(),scans.begin()+jump+1);
	return result;
    }
    // Split target at local maxima
    Target splitLocalMaxima() {
	assert(hits.size()>=2);
	int maxima=1; // Maxima at this point
	float depth=hits[maxima].norm() - std::max(hits[maxima-1].norm(),hits[maxima+1].norm());
	for (int i=1;i<hits.size()-1;i++) {
	    float tmpdepth=hits[i].norm() - std::max(hits[i-1].norm(),hits[i+1].norm());
	    if (tmpdepth>depth) {
		maxima=i;
		depth=tmpdepth;
	    }
	}
	int jump;
	// Check which side should get maxima
	if (hits[maxima-1].norm() < hits[maxima+1].norm())
	    jump=maxima-1;
	else
	    jump=maxima;

	dbg("World.makeAssignments",3) << "Split target into 2 at local maxima with depth " << depth << " at " << scans[0] << "-" << scans[jump] << " and " << scans[jump+1] << "-" << scans.back() << std::endl;
	Target result;
	for (int i=jump+1;i<hits.size();i++)
	    result.append(scans[i],hits[i]);
	hits=std::vector<Point>(hits.begin(),hits.begin()+jump+1);
	scans=std::vector<int>(scans.begin(),scans.begin()+jump+1);
	return result;
    }
    void setAssignments(std::vector<int> &assignments, std::vector<unsigned int> &legassigned, int assignedPerson, int assignedLeg)  {
	for (int i=0;i<scans.size();i++) {
	    assignments[scans[i]]=assignedPerson;
	    legassigned[scans[i]]=assignedLeg;
	}
	assigned=true;
    }
    int size() const { return scans.size(); }
    bool isAssigned() const { return assigned; }
    // Compute likelihood of least likely one of being background
    float minBgLike(const std::vector<float> &bglike) const { 
	float result=bglike[scans.front()];
	for (int i=1;i<scans.size();i++) 
	    result=std::min(result,bglike[scans[i]]);
	return result;
    }
    // Check if target would have a local maximum of size >= minsize if pt is added
    float getMaximaDepth() const {
	float depth=0;
	for (int i=0;i<hits.size();i++)
	    for (int j=i+1;j<hits.size();j++) {
		if (hits[j].norm()<depth+hits[i].norm())
		    continue;
		for (int k=j+1;k<hits.size();k++)
		    depth=std::max(depth,hits[j].norm()-std::max(hits[i].norm(),hits[k].norm()));
	    }
	return depth;
    }
    
};

void World::makeAssignments(const Vis &vis, float entrylike) {
    dbg("World.makeAssignments",3) << "makeAssignments(entrylike= " << entrylike << ")" << std::endl;
    const SickIO *sick=vis.getSick();
    const unsigned int *range=sick->getRange(0);
    // Build a list of targets that group together the hits
    std::vector<Target> targets;
    int nassigned=0;
    int nentries=0;
    int nbg=0;
    assignments.assign(sick->getNumMeasurements(),-2);	// Default everything to entries
    legassigned.assign(sick->getNumMeasurements(),0);
    float bglikethresh=log(0.5*UNITSPERM);

    for (unsigned int f=0;f<sick->getNumMeasurements();f++) {
	if (bglike[f]>bglikethresh) {
	    //dbg("World.makeAssignments",8) << "Assigned scan " << f << " with range " << range[f] << " to background with bglike= " << bglike[f] << std::endl;
	    assignments[f]=-1;
	    nbg++;
	    continue;  // Probably background
	}
	dbg("World.makeAssignments",8) << "Processing scan " << f << " with bglike= " << bglike[f] << std::endl;
	bool assigned=false;
	for (int i=targets.size()-1;i>=0;i--) {
	    // Check if we can add this point to an existing target
	    if ((sick->getWorldPoint(f)-targets[i].lastHit()).norm()<=MAXLEGDIAM)  {
		// Check if all intervening scans have a shorter range (i.e. we jumping over a near obstruction)
		bool canmerge=true;
		for (int j=targets[i].lastScan()+1;j<f;j++) {
		    dbg("World.makeAssignments",10) << "Comparing gap range[" << j << "] of " << range[j] << " with range[" << f << "] of " << range[f] << std::endl;
		    if (range[j] >= range[f]) {
			// Not closer, can' t do this merge
			dbg("World.makeAssignments",10) << "Can't merge scan " << f << " at range " << range[f] << " with target " << i << " since scan " << j << " has range " << range[j] << std::endl;
			canmerge=false;
			break;
		    }
		}
		if (!canmerge)
		    break;
		targets.back().append(f,sick->getWorldPoint(f));
		nassigned++;
		assigned=true;
		dbg("World.makeAssignments",4) << "Assigned scan " << f << " with range " << range[f] << " to target " << i << std::endl;
		break;
	    }
	}
	if (!assigned)  {
	    // Not a match to current target, add a new one
	    Target t;
	    t.append(f,sick->getWorldPoint(f));
	    targets.push_back(t);
	    dbg("World.makeAssignments",4) << "Assigned scan " << f << "with range " << range[f] << " to new target " << targets.size()-1 << std::endl;
	    nassigned++;
	}
    }
    dbg("World.makeAssignments",3) << "Assigned " << nassigned << " hits to " << targets.size() << " targets." << std::endl;
    // Split targets that are too wide
    for (int i=0;i<targets.size();i++) {
	if (targets[i].getMaximaDepth() >= TARGETMAXIMADEPTH)
	    targets.push_back(targets[i].splitLocalMaxima());
	if (targets[i].getWidth() > MAXLEGDIAM) {
	    targets.push_back(targets[i].split());
	}
    }

    // Now match targets with people, legs

    std::vector<int> legAssigned[2]; // Track assigned to leg, or -1 if not assigned

    for (int leg=0;leg<2;leg++)
	legAssigned[leg].assign(people.size(),-1);

    for (int pass=0;pass<people.size()*2;pass++) {
	float closest=1e10;
	int assignedTarget=-1;
	int assignedPerson=-1;
	int assignedLeg=-1;

	for (int t=0;t<targets.size();t++) {
	    if (targets[t].isAssigned())
		continue;
	    assert(targets[t].size()>0);
	    Point center=targets[t].getCenter();
	    for ( int i=0;i<people.size();i++) {
		for (int leg=0;leg<2;leg++) {
		    if (legAssigned[leg][i] != -1)
			continue;
		    float dist=(center-people[i].getLeg(leg).getPosition()).norm();
		    if (dist < closest) {
			closest=dist;
			assignedPerson=i;
			assignedLeg=leg;
			assignedTarget=t;
		    }
		}
	    }
	}
	if (closest<MAXASSIGNMENTDIST) {
	    dbg("World.makeAssignments",3) << "Assigning; closest person to  target " << assignedTarget << " is P" << people[assignedPerson].getID() << "." << assignedLeg << " with distance " << closest << std::endl;
	    targets[assignedTarget].setAssignments(assignments, legassigned, assignedPerson, assignedLeg);
	    legAssigned[assignedLeg][assignedPerson]=assignedTarget;  // Mark it as already assigned
	    if (!REASSIGNPOINTS) {
		int otherTarget=legAssigned[1-assignedLeg][assignedPerson];
		if (otherTarget != -1) {
		    // This is the second leg assigned, check if the assignment should be swapped
		    float currentd2 = pow((targets[assignedTarget].getCenter()-people[assignedPerson].getLeg(assignedLeg).getPosition()).norm(),2.0)+
			pow((targets[otherTarget].getCenter()-people[assignedPerson].getLeg(1-assignedLeg).getPosition()).norm(),2.0);
		    float swapd2 = pow((targets[otherTarget].getCenter()-people[assignedPerson].getLeg(assignedLeg).getPosition()).norm(),2.0)+pow((targets[assignedTarget].getCenter()-people[assignedPerson].getLeg(1-assignedLeg).getPosition()).norm(),2.0);
		    if (swapd2<currentd2) {
			dbg("World.makeAssignments",1) << "Swapping target assignments: P" << people[assignedPerson].getID() << "." << assignedLeg << " now gets target " << otherTarget << ", since sqd-dist with swap= " << swapd2 << " < " <<  currentd2 << std::endl;
			targets[assignedTarget].setAssignments(assignments, legassigned, assignedPerson, 1-assignedLeg);
			targets[otherTarget].setAssignments(assignments, legassigned, assignedPerson, assignedLeg);
		    } else {
			dbg("World.makeAssignments",3) << "Not swapping target assignments: P" << people[assignedPerson].getID() << "." << assignedLeg << " now gets target " << otherTarget << ", since sqd-dist with swap= " << swapd2 << " >  " <<  currentd2 << std::endl;
		    }
		}
	    }
	} else {
	    dbg("World.makeAssignments",3) << "Not assigning; closest target-person is  target " << assignedTarget << ", P" << people[assignedPerson].getID() << "." << assignedLeg << " with distance " << closest << std::endl;
	    break;
	}
    }

    // Unassigned ones
    for (int t=0;t<targets.size();t++) {
	if (!targets[t].isAssigned()) {
	    if (targets[t].minBgLike(bglike) > entrylike) {
		dbg("World.makeAssignments",3) << "Unassigned target " << t << " has minBgLike=" << targets[t].minBgLike(bglike) << ": assigning to background" << std::endl;
		targets[t].setAssignments(assignments, legassigned, -1,0);
		nbg+=targets[t].size();
	    } else {
		dbg("World.makeAssignments",3) << "Unassigned target " << t << " has minBgLike=" << targets[t].minBgLike(bglike) << ": assigning to entries" << std::endl;
		nentries+=targets[t].size();
	    }
	}
    }
    
    nassigned-=nentries;

    dbg("World.makeAssignments",3) << "Assigned " << nassigned << " points to existing people,  " << nbg  << " to background, and " << nentries << " to entries." << std::endl;
    if (nbg < nassigned+nentries) {
	dbg("World.makeAssignments",2) << "Only have " << nbg*1.0/(nbg+nassigned+nentries)*100 << "% of points assigned to background, using all points for update." << std::endl;
	sick->updateBackground(assignments,true);
    } else {
	// Update background only with points assumed to be background
	sick->updateBackground(assignments,false);
    }

    if (nentries>=MINCREATEHITS) {
	dbg("World.track",2) << "Have " << nentries << " non-background scan points unassigned." << std::endl;

	// Create a new track if we can find 2 points that are separated by ~meanlegsep
	float bestsep=1e10;
	int bestindices[2]={0,0};

	for (int t1=0;t1<targets.size();t1++) {
	    if (targets[t1].isAssigned() || targets[t1].size()<2)
		continue;
	    for (int t2=0;t2<targets.size();t2++) {
		if (targets[t2].isAssigned() || targets[t2].size()<2)
		    continue;
		float dist=(targets[t1].getCenter()-targets[t2].getCenter()).norm();
		dbg("World.track",4) << "Unassigned targets " << t1 << " and " << t2 << " are separated by " << dist << std::endl;
		if (dist>=MINLEGSEP && dist <= MAXLEGSEP && fabs(dist-MEANLEGSEP)<fabs(bestsep-MEANLEGSEP) ) {
		    bestsep=dist;
		    dbg("World.track",3) << "Unassigned targets " << t1 << " and " << t2 << " are best separation so far at " << bestsep << std::endl;
		    bestindices[0]=t1;
		    bestindices[1]=t2;
		}
	    }
	}
	if (bestsep<=MAXLEGSEP) {
	    dbg("World.track",1) << "Creating an initial track using targets " << bestindices[0] << "," << bestindices[1] << " with separation " << bestsep << std::endl;
	    Point l1=targets[bestindices[0]].getCenter();
	    Point l2=targets[bestindices[1]].getCenter();
	    people.add(l1,l2);
	    targets[bestindices[0]].setAssignments(assignments,legassigned,people.size()-1,0);
	    targets[bestindices[1]].setAssignments(assignments,legassigned,people.size()-1,1);
	} else {
	    dbg("World.track",2) << "Not creating a track - no pair appropriately spaced" << std::endl;
	}
    }
}

void World::track( const Vis &vis, int frame, float fps,double elapsed) {
    int nsteps;
    if (lastframe>0 && frame>lastframe)
	nsteps=frame-lastframe;
    else {
	dbg("World.track",1) << "lastframe=" << lastframe << ", frame=" << frame << ": setting nsteps=1" << std::endl;
	nsteps=1;
    }
    lastframe=frame;

    // Update existing tracks with next prediction
    for (unsigned int i=0;i<people.size();i++)
	people[i].predict(nsteps,fps);

    int num_measurements=vis.getSick()->getNumMeasurements();
    float entryprob=1-exp(-ENTRYRATE/60.0*nsteps/fps);
    float entrylike=log(entryprob/num_measurements*10);  // Like that a scan is a new entry (assumes 10 hits on avg)
    dbg("World.track",2) << "Tracking frame " << frame << std::endl;

    // Calculate background likelihoods
    bglike=vis.getSick()->getBackground().like(vis,*this);

    if (!vis.getSick()->getBackground().isInitializing())
	// Map scans to tracks, and update background
	makeAssignments(vis,entrylike);
    else
	// Update all points for background
	vis.getSick()->updateBackground(assignments,true);
	

    // Implement assignment
    for (unsigned int p=0;p<people.size();p++) {

	// Build list of points assigned to this person
	std::vector<int> fs[2];
	if (REASSIGNPOINTS) {
	    std::vector<float> ldiff;
	    // Check differential effect of assigning to one or the other
	    for (unsigned int j=0;j<assignments.size();j++) 
		if (assignments[j]==(int)p) {
		    // Redo assignments to which leg based on observertion
		    Point sickpt=vis.getSick()->getWorldPoint(j);
		    float l1=people[p].getObsLike(sickpt,0,vis.getSick()->getFrame());
		    float l2=people[p].getObsLike(sickpt,1,vis.getSick()->getFrame());
		    ldiff.push_back(l1-l2);
		    fs[0].push_back(j);
		}
	    // Check all possible splits into 1 or 2 contiguous groups
	    float maxldiff=0;
	    int bestsplit=0;
	    for (unsigned int j=0;j<ldiff.size()+1;j++)  {
		float ltotal=std::accumulate(ldiff.begin(),ldiff.begin()+j,0)-std::accumulate(ldiff.begin()+j,ldiff.end(),0);
		if (fabs(ltotal)>fabs(maxldiff)) {
		    maxldiff=ltotal;
		    bestsplit=j;
		}
	    }
	    dbg("World.track",5) << "Best split of points assigned to person P" <<  people[p].getID() << " (" << fs[0] <<  ") is at " << bestsplit << " with likelihood difference " << maxldiff << std::endl;
	    if (maxldiff<0) {
		fs[1].assign(fs[0].begin(),fs[0].begin()+bestsplit);
		fs[0].erase(fs[0].begin(),fs[0].begin()+bestsplit);
	    } else {
		fs[1].assign(fs[0].begin()+bestsplit,fs[0].end());
		fs[0].erase(fs[0].begin()+bestsplit,fs[0].end());
	    }

	} else {  /* ! REASSIGNPOINTS */
	    for (unsigned int j=0;j<assignments.size();j++)
		if (assignments[j]==(int)p)
		    fs[legassigned[j]].push_back(j);

	    // Reorganize split between two legs if needed (e.g. frame 958)
	    // TODO -- these splits don't take into account which prior position is closer so they result in leg swaps sometimes (e.g. frame 958 of *2923.ferec)
	    bool split=false;
	    for (int f=0;f<2;f++) {
		if (fs[f].size()>0 && fs[1-f].size()==0) {
		    // One empty, maybe we can move some over at a gap
		    if (fs[f].back()-fs[f].front()+1 != (int)fs[f].size()) {
			// Has a gap
			for (unsigned int i=1;i<fs[f].size();i++) 
			    // Gap must be a more distant range rather than shadowing
			    if (fs[f][i]-fs[f][i-1] != 1 && vis.getSick()->getRange(0)[fs[f][i]-1]>vis.getSick()->getRange(0)[fs[f][i]]) {
				for (unsigned int j=i;j<fs[f].size();j++)
				    fs[1-f].push_back(fs[f][j]);
				fs[f].resize(i);
				dbg("World.track",2) << "Splitting preliminary assignment for ID " << people[p].getID() << " at a gap into " << fs[0] << ", " << fs[1] << std::endl;
				split=true;
				break;
			    }
		    }
		}
		if (fs[f].size()>0 && fs[1-f].size()==0) {
		    // Still one empty, look for a large range jump
		    for (unsigned int i=1;i<fs[f].size();i++) { 
			float rjump=(vis.getSick()->getWorldPoint(fs[f][i])-vis.getSick()->getWorldPoint(fs[f][i-1])).norm();
			if (rjump>INITLEGDIAM/2) {
			    for (unsigned int j=i;j<fs[f].size();j++)
				fs[1-f].push_back(fs[f][j]);
			    fs[f].resize(i);
			    dbg("World.track",2) << "Splitting preliminary assignment for ID " << people[p].getID() << " at a range jump of " << rjump << " into " << fs[0] << ", " << fs[1] << std::endl;
			    split=true;
			    break;
			}
		    }
		}
	    }
	    if (split) {
		// Check if they need to be swapped
		float swaplike=0;   // Compute difference in likelihood between matching as-is vs. matching swapped
		for (int f=0;f<2;f++)
		    for (int leg=0;leg<2;leg++) {
			for (unsigned int i=0;i<fs[f].size();i++) {
			    Point sickpt=vis.getSick()->getWorldPoint(fs[f][i]);
			    float like = people[p].getObsLike(sickpt,leg,vis.getSick()->getFrame());
			    if (leg==f)
				swaplike+=like;
			    else
				swaplike-=like;
			}
		    }
		dbg("World.track",2) << "Checking if should swap leg assignments, swaplike=" << swaplike << std::endl;
		if (swaplike < 0) {
		    std::swap(fs[0],fs[1]);
		    dbg("World.track",2) << "Swapped assignment for ID " << people[p].getID() << " into " << fs[0] << ", " << fs[1] << std::endl;
		}
	    }
	}

	people[p].update(vis,bglike,fs,nsteps,fps);
    }

    // Delete lost people
    for (unsigned int i=0;i<people.size();i++)
	if (people[i].isDead()) {
	    dbg("World.track",1) << "Erasing people[" << i << "] = person: " << people[i] << std::endl;
	    people.erase(i);
	    i--;
	}

    // Track groups
    groups.update(people,elapsed);

    if (DebugCheck("World.track",2) && people.size() > 0) {
	dbg("World.track",2)  << "People at end of frame " <<  lastframe << ":" << std::endl;
	for (unsigned int i=0;i<people.size();i++)
	    dbg("World.track",2)  << people[i] << std::endl;
    }

}
        
void World::sendMessages(Destinations &dests, double elapsed) {
    dbg("World.sendMessages",5) << "ndest=" << dests.size() << std::endl;
    std::vector<lo_address> addr;
    for (int i=0;i<dests.size();i++) {
	if (dests.getFailCount(i) > 0) {
	    if (lastframe%3000 == 0)
		// Return each minute
		std::cerr << "Retrying previously failed destination " << dests.getHost(i) << ":" << dests.getPort(i) << std::endl;
	    else
		continue;
	}
	char cbuf[10];
	dbg("World.sendMessages",6) << "Sending messages to " << dests.getHost(i) << ":" << dests.getPort(i) << std::endl;
	sprintf(cbuf,"%d",dests.getPort(i));
	lo_address a=lo_address_new(dests.getHost(i), cbuf);
	if (lo_send(a,"/pf/frame","i",lastframe) < 0) {
	    std::cerr << "Failed send of /pf/frame to " << lo_address_get_url(a) << std::endl;
	    dests.setFailed(i);
	    continue;
	}
	addr.push_back(a);
	if (dests.getFailCount(i) > 0) {
	    std::cerr << "Previously failed destination, " << dests.getHost(i) << ":" << dests.getPort(i) << " is accepting packets again" << std::endl;
	}
	dests.setSucceeded(i);
    }

    // Handle entries
    std::set<int>exitids = lastid;
    unsigned int priornpeople=lastid.size();
    unsigned activePeople=0;
    for (int pi=0;pi<people.size();pi++){
	if (people[pi].getAge() >= AGETHRESHOLD) {
	    activePeople++;
	    exitids.erase(people[pi].getID());
	    if ( lastid.count(people[pi].getID()) == 0) {
		for (unsigned int i=0;i<addr.size();i++)
		    lo_send(addr[i],"/pf/entry","ifii",lastframe,elapsed,people[pi].getID(),people[pi].getChannel());
	    }
	    lastid.insert(people[pi].getID());
	}
    }

    // Handle exits
    for (std::set<int>::iterator p=exitids.begin();p!=exitids.end();p++) {
	for (unsigned int i=0;i<addr.size();i++)
	    lo_send(addr[i],"/pf/exit","ifi",lastframe,elapsed,*p);
	lastid.erase(*p);
    }

    // Current size
    if (activePeople != priornpeople)
	for (unsigned int i=0;i<addr.size();i++)
	    lo_send(addr[i],"/pf/set/npeople","i",activePeople);

    if (groups.size() != priorngroups) {
	for (unsigned int i=0;i<addr.size();i++)
	    lo_send(addr[i],"/pf/set/ngroups","i",groups.size());
	priorngroups=groups.size();
    }

    // Updates
    for (int pi=0;pi<people.size();pi++){
	if (people[pi].getAge() >= AGETHRESHOLD)
	    for (unsigned int i=0;i<addr.size();i++)
		people[pi].sendMessages(addr[i],lastframe,elapsed);
    }
    // Groups
    for (unsigned int i=0;i<addr.size();i++)
	groups.sendMessages(addr[i],lastframe,elapsed);

    // Geo
    Point center;  // Center of all participants
    int ccnt=0;
    for (int pi=0;pi<people.size();pi++)
	if (people[pi].getAge() >= AGETHRESHOLD) {
	    center=center+people[pi].getPosition();
	    ccnt++;
	}
    if (ccnt>0) {
	center=center/ccnt;
	for (int pi=0;pi<people.size();pi++) {
	    if (people[pi].getAge() < AGETHRESHOLD)
		continue;

	    float centerDist=(people[pi].getPosition()-center).norm();
	    float otherDist=-1;
	    for (int  pi2=0;pi2<people.size();pi2++) {
		float dist = (people[pi2].getPosition()-people[pi].getPosition()).norm();
		if (dist>0.001 && (dist<otherDist || otherDist==-1))
		    otherDist=dist;
	    }
	    float exitDist=distanceToBoundary(people[pi].getPosition());

	    if (otherDist>0)
		otherDist/=UNITSPERM;
	    for (unsigned int i=0;i<addr.size();i++)
		if (lo_send(addr[i], "/pf/geo","iifff",lastframe,people[pi].getID(),centerDist/UNITSPERM,otherDist,exitDist/UNITSPERM) < 0) {
		    std::cerr << "Failed send of /pf/geo to " << lo_address_get_url(addr[i]) << std::endl;
		    return;
		}
	}
    }

    // Done!
    for (unsigned int i=0;i<addr.size();i++)
	lo_address_free(addr[i]);
}

#ifdef MATLAB
mxArray *World::convertToMX() const {
    const char *fieldnames[]={"tracks","npeople","assignments","bglike","bestlike","bounds"};
    mxArray *world = mxCreateStructMatrix(1,1,sizeof(fieldnames)/sizeof(fieldnames[0]),fieldnames);

    mxArray *pNpeople = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    *(int *)mxGetPr(pNpeople) = people.size();
    mxSetField(world,0,"npeople",pNpeople);

    mxArray *pAssignments = mxCreateNumericMatrix(assignments.size(),2,mxINT32_CLASS,mxREAL);
    int *idata = (int *)mxGetPr(pAssignments);
    assert(legassigned.size()==assignments.size());
    for (unsigned int i=0;i<assignments.size();i++) 
	*idata++=assignments[i];
    for (unsigned int i=0;i<assignments.size();i++) 
	*idata++=legassigned[i];
    mxSetField(world,0,"assignments",pAssignments);

    mxArray *pBestlike = mxCreateDoubleMatrix(bestlike.size(),2,mxREAL);
    double *data = mxGetPr(pBestlike);
    for (unsigned int i=0;i<bestlike.size();i++)
	*data++=bestlike[i];
    mxSetField(world,0,"bestlike",pBestlike);

    mxArray *pBglike = mxCreateDoubleMatrix(1,bglike.size(),mxREAL);
    data=mxGetPr(pBglike);
    for (unsigned int i=0;i<bglike.size();i++)
	*data++=bglike[i];
    mxSetField(world,0,"bglike",pBglike);

    mxArray *pBounds = mxCreateDoubleMatrix(1,4,mxREAL);
    data=mxGetPr(pBounds);
    *data++=minx;
    *data++=maxx;
    *data++=miny;
    *data++=maxy;
    mxSetField(world,0,"bounds",pBounds);

    const char *pfieldnames[]={"id","position","legs","predictedlegs","prevlegs","legvelocity","scanpts","persposvar","posvar","prevposvar","velocity","legdiam","leftness","maxlike","like","minval","maxval","age","consecutiveInvisibleCount","totalVisibleCount"};
    mxArray *pPeople;
    if ((pPeople = mxCreateStructMatrix(1,people.size(),sizeof(pfieldnames)/sizeof(pfieldnames[0]),pfieldnames)) == NULL) {
	fprintf(stderr,"Unable to create people matrix of size (1,%d)\n",people.size());
    }

    for (unsigned int i=0;i<people.size();i++) {
	people[i].addToMX(pPeople,i);
    }

    if (people.size()>0 && mxSetClassName(pPeople,"Person")) {
	fprintf(stderr,"Unable to convert people to a Matlab class\n");
    }

    mxSetField(world,0,"tracks",pPeople);					      

    if (mxSetClassName(world,"World")) {
	fprintf(stderr,"Unable to convert world to a Matlab class\n");
    }

    return world;
}
#endif
