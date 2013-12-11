Pulsefield pf;
pf.initialize();

while (true) {
      Pulsefield.changed=> now;
      <<< "min=[",pf.minx,",",pf.miny,"],  max=[",pf.maxx,",",pf.maxy,"], npeople=",pf.npeople,", running=",pf.running >>>;
      for (0=>int i;i<pf.npeople;i++) {
	  pf.people[i] @=> Person p;
	  <<<"  Person ",p.id," at (",p.x,",",p.y,"), vel=(",p.vx,",",p.vy,"), diam=(",p.d1,",",p.d2,"), channel=",p.channel,", group=",p.group," with ",p.ngroup," person(s)">>>;
      }
 }

      
