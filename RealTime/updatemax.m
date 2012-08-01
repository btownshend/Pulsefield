% Update MAX with a new set of locations for the targets
function ok=updatemax(p,snap,prevsnap)
% Clear all old values since we don't know the specific ones to replace
ok=sendtomax('/coll',{'clear'});

if nargin<1
  return;
end

% Send new values
for i=1:length(snap.hypo)
  pos=snap.hypo(i).pos;
  ok=ok&sendtomax('/coll',{'symbol',int32(i),single(pos(1)),single(pos(2))});
end

% Update LCD
lcdsize=400;
for i=1:length(snap.hypo)
  pos=snap.hypo(i).pos;
   lp=int32((pos+2.5)*lcdsize/5);
   sz=2;
   lp=lp-sz/2;
   le=lp+sz;
   col=id2color(snap.hypo(i).id);
   if all(col==127)
     col=[0 0 0];
   end
   col=int32(col*255);
   ok=ok&sendtomax('/lcd',{'frgb',col(1),col(2),col(3)});
   ok=ok&sendtomax('/lcd',{'paintoval',lp(1),lp(2),le(1),le(2)});
end	

% Update step sequencer
nsteps=8;
rng=[-2,2];
stepsize=diff(rng)/(nsteps-1);
stepedges=rng(1)+(0:nsteps-2)*stepsize;
velocity=127;
duration=120;

if length(snap.hypo)>0
  pitches=[]; step=[]; channel=[];
  for i=1:length(snap.hypo)
    step(i)=max([1,find(snap.hypo(i).pos(1)>stepedges)]);
    pitches(i)=pos2pitch(snap.hypo(i).pos(2));
    channel(i)=i;
  end
  [step,ord]=sort(step);
  pitches=pitches(ord);
  channel=channel(ord);
  pv=zeros(1,nsteps);
  for i=1:nsteps
    sel=find(step==i);
    if length(sel)>1
      step(sel(2:end))=step(sel(2:end))+1;
      sel=find(step==i);
    end
    if isempty(sel)
      p=0;
      ch=0;
    else
      p=pitches(sel);
      ch=channel(sel);
    end
      
    if p>0
      ok=ok&sendtomax('/seq',{'step',int32(i),int32(p), int32(velocity), int32(duration), int32(ch)});
    else
      ok=ok&sendtomax('/seq',{'step',int32(i),int32(p), int32(0), int32(duration), int32(ch)});
    end
    %    if p>0
    %      fprintf('step %d: pitch=%d\n', i, p);
    % 	 end
  end
  disprange=[min([pitches(pitches>0)-4,36]),max([pitches+4,86])];
  ok=ok&sendtomax('/seq',{'zoom',int32(disprange(1)),int32(disprange(2))});
  ok=ok&sendtomax('/seq',{'loop',int32(1),int32(nsteps)});
  ok=ok&sendtomax('/seq',{'active',int32(1)});
else
  ok=ok&sendtomax('/seq',{'active',int32(0)});
end

% Map a position in meters to a MIDI pitch val
function pitch=pos2pitch(pos)
pitch=int32(60+pos*10);   % 10 notes/meter centered at middle-C
