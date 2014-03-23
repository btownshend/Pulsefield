fps=50;
if ~exist('oldsnap','var')
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
end

%tracker=multiObjectTracking();
tracker=World();
bg=[];
iswaiting=false;
snap=[];

while true
  if exist('oldsnap','var')
    % Reprocess oldsnap
    if length(snap)>=length(oldsnap)
      break;
    end
    vis=oldsnap(length(snap)+1).vis;
  else
    vis=sickrcvr('debug',0,'skipstale',false);
  end
    
  if isempty(vis)
    if iswaiting
      fprintf('.');
    else
      fprintf('Waiting for data from frontend.');

      if length(snap)>0
        diagnostic(snap);
      end
      iswaiting=true;
    end
    pause(0.01);
    continue;
  end
  if iswaiting
    fprintf('done\n');
  end
  iswaiting=false;
  if isfield(vis,'reflect')
    vis.reflect=vis.reflect(:,1,:);
  end

  fprintf('\n*** Snap %d\n', length(snap)+1);
  
  if isempty(bg)
    bg=Background(vis);
  else
    bg.update(vis);
  end

  vis=classify(vis,bg);
  vis=splitclasses(vis,0.25);  % TODO: this and other constants should be collected up somewhere
  %  vis=joinlegs(vis);
  %  vis=maketargets(vis);
  if isempty(snap)
    npredict=1;
  else
    npredict=vis.cframe-snap(end).vis.cframe;
  end
  if npredict<0
    % Probably due to frontend playing back a file in a loop
    fprintf('Got frame %d after frame %d ... quittting\n',vis.cframe,snap(end).vis.cframe);
    break;
  end
  if npredict>1
    fprintf('Skipping ahead %d frames\n', npredict);
  end
  tracker.update(vis,npredict,fps);
  snap=[snap,struct('vis',vis,'bg',bg,'tracker',tracker.clone())];
  if length(snap)>1
    sendosc({'VD'},snap(end),snap(end-1));
  else
    sendosc({'VD'},snap(end));
  end
  
  if mod(length(snap),100)==0
    fprintf('%d...',length(snap));
  end
end
