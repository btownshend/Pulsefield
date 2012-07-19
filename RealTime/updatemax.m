% Update MAX with a new set of locations for the targets
function updatemax(snap,prevsnap)
% Clear all old values since we don't know the specific ones to replace
sendtomax('/coll',{'clear'});

if nargin<1
  return;
end

% Send new values
for i=1:length(snap.hypo)
  pos=snap.hypo(i).pos;
  sendtomax('/coll',{'symbol',int32(i),single(pos(1)),single(pos(2))});
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
      sendtomax('/seq',{'step',int32(i),int32(p), int32(velocity), int32(duration), int32(ch)});
    else
      sendtomax('/seq',{'step',int32(i),int32(p), int32(0), int32(duration), int32(ch)});
    end
    if p>0
      fprintf('step %d: pitch=%d\n', i, p);
    end
  end
  disprange=[min([pitches(pitches>0)-4,36]),max([pitches+4,86])];
  sendtomax('/seq',{'zoom',int32(disprange(1)),int32(disprange(2))});
  sendtomax('/seq',{'loop',int32(1),int32(nsteps)});
  sendtomax('/seq',{'active',int32(1)});
else
  sendtomax('/seq',{'active',int32(0)});
end

% Map a position in meters to a MIDI pitch val
function pitch=pos2pitch(pos)
pitch=int32(60+pos*10);   % 10 notes/meter centered at middle-C
