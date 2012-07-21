% mainloop - used for both replays and real time
% Inputs: recvis struct
%	  timedreplay - true to run replay at same speed as original
%	  plots - set of plots to do ('stats','hypo','ana','vis')
function recvis=mainloop(recvis,timedreplay,plots)
if nargin<2
  timedreplay=0;   % Set to 1 to replay at same pacing as recording
end
if nargin<3
  plots={};
end
userecvis=length(recvis.vis)>0;

if ismember('hypo',plots)
  setfig('plothypo');clf;
  plotlayout(recvis.layout,0)
  hold on;
end
samp=1;
snap={};
starttime=now;
suppressuntil=0;
while ~userecvis || samp<=length(recvis.vis)
  tic;
    
  elapsed=(now-starttime)*24*3600;
  if userecvis
    vis=recvis.vis(samp);
    if timedreplay
      sleeptime=((vis.when-recvis.vis(1).when)-(now-starttime))*24*3600;
      if sleeptime>0
        pause(sleeptime);
      elseif sleeptime<-1
        fprintf('Running behind by %.1f seconds\n', -sleeptime);
      end
    end
  else
    %    vis=getvisible(recvis.p,0);
    vis=getvisfrompipe(recvis.p,recvis.randseed);
    recvis.vis=[recvis.vis,vis];
  end
  if ismember('vis',plots)
    plotvisible(recvis.p,vis);
  end
  % Check for bad data
  transitions=sum(abs(diff(vis.v'))==1);
  if any(transitions>20) && samp>=suppressuntil
    fprintf('Too many LED blockage transitions at snapshot %d: %s\n', samp, sprintf('%d ',transitions));
    suppressuntil=samp+50;
  end

  % Analyze data to estimate position of targets using layout
  pflg=0;
  if ismember('ana',plots)
    pflg=1;
  end
  if ismember('rays',plots)
    pflg=2;
  end
  snap{samp}=analyze(recvis.p,recvis.layout,vis.v,recvis.rays,pflg);
  snap{samp}.when=vis.when;
  if samp==1
    % hypo{samp}=inithypo(snap{1},recvis.rays.imap);
    snap{1}=inittgthypo(snap{1});
  else
    snap{samp}=updatetgthypo(recvis.layout,snap{samp-1},snap{samp});
  end
  
  if ~userecvis
    if samp==1
      recvis.snap=snap{samp};
    else
      recvis.snap=[recvis.snap,snap{samp}];
    end
  end

  if ismember('hypo',plots)
    setfig('plothypo');
    foundk=zeros(length(snap{samp}.hypo),1);

    if samp>1
      foundj=zeros(length(snap{samp-1}.hypo),1);
      for j=1:length(snap{samp-1}.hypo)
        hj=snap{samp-1}.hypo(j);
        for k=1:length(snap{samp}.hypo)
          hk=snap{samp}.hypo(k);
          if hj.id==hk.id
            % Join nearby positions
            col=id2color(hj.id);
            plot([hj.pos(1),hk.pos(1)],[hj.pos(2),hk.pos(2)],'Color',col);
            foundj(j)=1;
            foundk(k)=1;
          end
        end
      end
      for j=1:length(snap{samp-1}.hypo)
        if ~foundj(j)
          hj=snap{samp-1}.hypo(j);
          col=id2color(hj.id);
          plot(hj.pos(1),hj.pos(2),'x','Color',col);
        end
      end
    end
    for k=1:length(snap{samp}.hypo)
      if ~foundk(k)
        hk=snap{samp}.hypo(k);
        col=id2color(hk.id);
        plot(hk.pos(1),hk.pos(2),'o','Color',col);
      end
    end
  end

  % Display coords
%  fprintf('%3d@%4.1f ',samp,elapsed);
  for j=1:length(snap{samp}.tgts)
    tj=snap{samp}.tgts(j).pos;
    fprintf('T%d:(%.2f,%.2f) ',j,tj);
  end
  for j=1:length(snap{samp}.hypo)
    hj=snap{samp}.hypo(j);
    if samp>1 && length(snap{samp-1}.hypo)>=j && snap{samp-1}.hypo(j).id==hj.id
      hk=snap{samp-1}.hypo(j);
    end
    fprintf('H%d:(%.2f,%.2f)@%.1f m/s ',hj.id,hj.pos,hj.speed);
  end
  fprintf('\n');
  
  if samp>1
    updatemax(snap{samp},snap{samp-1});
  else
    updatemax(snap{samp});
  end
  if ~userecvis
    updateleds(recvis.p,recvis.layout,snap{samp});
  end
  loopend=toc;
  %  fprintf('Loop time = %.2f seconds\n', loopend);
  if ~isempty(plots)
    pause(0.01);   % Give time for figures to refresh
  end
  samp=samp+1;
end
% Turn off any remaining notes
updatemax();

if ismember('stats',plots)
  plotstats(snap);
end