function sendosc(dests,snap,prevsnap)
global firsttime

if nargin<3 || firsttime==0
  firsttime=snap.vis.when;
end
tm=(snap.vis.when-firsttime)*24*3600;

bxy=range2xy(snap.bg.angle,snap.bg.range(1,:));

if mod(snap.vis.frame,100)==0
  oscmsgout(dests,'/pf/set/minx',{min(bxy(:,1))});
  oscmsgout(dests,'/pf/set/maxx',{max(bxy(:,2))});
  oscmsgout(dests,'/pf/set/miny',{min(bxy(:,1))});
  oscmsgout(dests,'/pf/set/maxy',{max(bxy(:,2))});
end

oscmsgout(dests,'/pf/frame',{int32(snap.vis.frame)});

cid=arrayfun(@(z) z.id, snap.tracker.tracks);
if nargin<3
  pid=[];
else
  pid=arrayfun(@(z) z.id, prevsnap.tracker.tracks);
end

for i=1:length(snap.tracker.tracks)
  t=snap.tracker.tracks(i);
  if t.age==1
    oscmsgout(dests,'/pf/entry',{int32(snap.vis.frame),tm,int32(t.id),int32(0)});
  end
  legs=t.legs;
  mmaxes=[norm(diff(legs,1)),0]+t.legdiam;
  oscmsgout(dests,'/pf/update',{int32(snap.vis.frame),tm,int32(t.id),t.position(1),t.position(2),t.velocity(1),t.velocity(2),mmaxes(1),mmaxes(2),int32(0),int32(1),int32(0)});
end

lost=setdiff(pid,cid);
for i=1:length(lost)
  oscmsgout(dests,'/pf/exit',{int32(snap.vis.frame),tm,int32(lost(i))});
end

if length(pid)~=length(cid)
  oscmsgout(dests,'/pf/set/npeople',{int32(length(cid))});
end

  
  