p=struct();
p.analsysisparams=analysissetup();
p.lidar=struct('id',1);
[~,myport]=getsubsysaddr('MPV');
fprintf('Instructing frontend to use port %d to send us msgs\n', myport);
oscmsgout('FE','/vis/dest/add/port',{myport});
ok=oscping('FE','MPV');
if ~felidarctl(p,'ping') && ~felidarctl(p,'start')
  error('Failed to start front end');
end

if ~ok
  error('Failed ping of front end');
end

oscmsgout('FE','/vis/get/reflect',{uint32(0)});
oscmsgout('FE','/vis/set/echoes',{uint32(1)});

tracker=multiObjectTracking();
im=255*ones(600,600,3,'uint8');
winbounds=[-3,3,-0.5,5];

bg=[];
fnum=[];
fps=5;  % Video display rate
lastacquired=0;
ftime=(1/fps)/3600/24;
iswaiting=false;
while true
  newvis=sickrcvr('debug',0);
  if isempty(newvis)
    if iswaiting
      fprintf('.');
    else
      fprintf('Waiting for data from frontend.');
      iswaiting=true;
    end
    continue;
  end
  if iswaiting
    fprintf('done\n');
  end
  iswaiting=false;
  newvis.range=newvis.range(:,1,:);
  if isfield(newvis,'reflect')
    newvis.reflect=newvis.reflect(:,1,:);
  end

  bg=updatebg(bg,newvis);
  newvis.targets=classify(newvis,bg);

  tracker.update(newvis.targets.pos,newvis.targets.bbox);
  if newvis.acquired>lastacquired+ftime
    im2=vis2image(newvis,im,winbounds,0);
    im3=vis2image(bg,im2,winbounds,1);
    tracker.displayTrackingResults(im3,winbounds);
    lastacquired=newvis.acquired;
  end
  fnum(end+1)=newvis.cframe;
end
