% Replay a recorded set of movements stored in recvis
function replayrecvis=replay(varargin)
defaults=struct('timed',false,'plots',{{}},'oscdests',{{'MAX','LD','TO'}},'frames',[nan,nan],'plothypo',false,'plotvis',false, 'plotanalyze',0);
args=processargs(defaults,varargin);

global recvis
if ~exist('recvis','var') || isempty(recvis.vis)
  error('Nothing to replay -- need to load recvis');
end

if length(recvis.vis)>length(recvis.snap)
  fprintf('Truncating recvis.vis from %d to %d samples to match recvis.snaps\n', length(recvis.vis), length(recvis.snap));
  recvis.vis=recvis.vis(1:length(recvis.snap));
end

fprintf('Replaying session from %s to %s\n', datestr(recvis.vis(1).when),datestr(recvis.vis(end).when));
if isfield(recvis,'note')
  fprintf('Notes: %s\n',recvis.note);
end

timedreplay=args.timed;   % Set to 1 to replay at same pacing as recording

osclog('close');   % Close any old logs open
oscclose();        % Close any old clients

% Run sanity tests
if ~oscloopback('MPO') || ~oscloopback('MPL')
  fprintf('Loopback test failed\n');
  return;
end
oscclose();  % Don't want these all in the default destinations

% Setup destination for outgoing OSC messages
recvis.p.oscdests=args.oscdests;

if args.plothypo
  setfig('plothypo');clf;
  subplot(221);
  plotlayout(recvis.p.layout,0)
  hold on;
  subplot(222);
  hold on;
  subplot(224);
  hold on;
end
samp=1;
starttime=now;
suppressuntil=0;
info=infoinit();
while samp<=min(length(recvis.vis),length(recvis.snap))
  vis=recvis.vis(samp);
  simulskew = now-vis.whenrcvd;     % Delay in simulation from when it really happened
  vis.whenrcvd=now;
  if timedreplay
    sleeptime=((vis.when-recvis.vis(1).when)-(now-starttime))*24*3600;
    if sleeptime>0
      pause(sleeptime);
    elseif sleeptime<-1
      fprintf('Running behind by %.1f seconds\n', -sleeptime);
    end
  end
  if args.plotvis
    plotvisible(recvis.p,vis);
  end
  % Check for bad data
  transitions=sum(abs(diff(vis.v'))==1);
  if any(transitions>20) && samp>=suppressuntil
    suppressuntil=samp+50;
    fprintf('Too many LED blockage edges at snapshot %d: %s (ignoring until samp %d)\n', samp, sprintf('%d ',transitions),suppressuntil);
  end

  % Analyze data to estimate position of targets using layout
  snap{samp}=analyze(recvis.p,vis.v,args.plotanalyze);
  snap{samp}.when=vis.when;
  snap{samp}.samp=samp;
  if samp==1
    snap{1}=inittgthypo(snap{1});
  else
    snap{samp}=updatetgthypo(recvis.p.layout,snap{samp-1},snap{samp},samp);
  end

  snap{samp}.whendone=now-simulskew;

  if args.plothypo && ~isempty(snap{samp}.hypo)
    setfig('plothypo');
    subplot(221);
    foundk=zeros(length(snap{samp}.hypo),1);

    if samp>1
      [ids,pid,cid]=intersect([snap{samp-1}.hypo.id],[snap{samp}.hypo.id]);
      for j=1:length(ids)
        hj=snap{samp-1}.hypo(pid(j));
        hk=snap{samp}.hypo(cid(j)); 
        plot([hj.pos(1),hk.pos(1)],[hj.pos(2),hk.pos(2)],'Color',id2color(ids(j),recvis.p.colors));
      end

      % Exits
      [~,exid]=setdiff([snap{samp-1}.hypo.id],[snap{samp}.hypo.id]);
      for j=1:length(exid)
        hj=snap{samp-1}.hypo(exid(j));
        plot(hj.pos(1),hj.pos(2),'x','Color',id2color(hj.id,recvis.p.colors));
      end

      % Entrances
      [~,enid]=setdiff([snap{samp}.hypo.id],[snap{samp-1}.hypo.id]);
      for j=1:length(enid)
        hj=snap{samp}.hypo(enid(j));
        plot(hj.pos(1),hj.pos(2),'o','Color',id2color(hj.id,recvis.p.colors));
      end

      subplot(222);
      for j=1:length(snap{samp}.tgts)
        tj=snap{samp}.tgts(j);
        plot([samp,samp],[min(tj.pixellist(:,1)),max(tj.pixellist(:,1))],'k'); 
        plot(samp,tj.pos(1),'.k');
      end
      for j=1:length(ids)
        hj=snap{samp-1}.hypo(pid(j));
        hk=snap{samp}.hypo(cid(j));
        plot([samp-1,samp],[hj.pos(1),hk.pos(1)],'Color',id2color(hj.id,recvis.p.colors));
      end

      subplot(224);
      for j=1:length(snap{samp}.tgts)
        tj=snap{samp}.tgts(j);
        plot([samp,samp],[min(tj.pixellist(:,2)),max(tj.pixellist(:,2))],'k');
        plot(samp,tj.pos(2),'.k');
      end
      for j=1:length(ids)
        hj=snap{samp-1}.hypo(pid(j));
        hk=snap{samp}.hypo(cid(j));
        plot([samp-1,samp],[hj.pos(2),hk.pos(2)],'Color',id2color(hj.id,recvis.p.colors));
      end
    end
  end
  
  
  % Display coords
  %  fprintf('%3d@%4.1f ',samp,(now-starttime)*24*3600);
  ptd=false;
  for j=1:length(snap{samp}.tgts)
    tj=snap{samp}.tgts(j).pos;
    fprintf('T%d:(%.2f,%.2f) ',j,tj);
    ptd=true;
  end
  for j=1:length(snap{samp}.hypo)
    hj=snap{samp}.hypo(j);
    if samp>1 && length(snap{samp-1}.hypo)>=j && snap{samp-1}.hypo(j).id==hj.id
      hk=snap{samp-1}.hypo(j);
    end
    fprintf('H%d:(%.2f,%.2f)@%.2f m/s ',hj.id,hj.pos,norm(hj.velocity));
    ptd=true;
  end
  if ptd
    fprintf('(%d) \n',samp);
  end
  
  info=oscincoming(recvis.p,info);
  
  if samp>1
    info=oscupdate(recvis.p,info,samp,snap{samp},snap{samp-1});
  else
    info=oscupdate(recvis.p,info,samp,snap{samp});
  end

  snap{samp}.whendone2=now-simulskew;

  % loopend=toc;
  % fprintf('Loop time = %.2f seconds\n', loopend);
  % tic
  pause(0.01);   % Give time for figures to refresh
  samp=samp+1;
end

% Turn off any remaining notes
oscupdate(recvis.p,info,samp);

% Copy in snap to new version
fprintf('Copying new analysis into replayrecvis\n');
replayrecvis=rmfield(recvis,'snap');
for i=1:length(snap)
  replayrecvis.snap(i)=snap{i};
end
if ~args.plothypo
  % Only if not already plotted as the replay ran
  plothypo(replayrecvis);
end
plotlatency(replayrecvis);
plotstats(replayrecvis);
% Can run analyze(recvis.p,recvis.vis(nnn).v,2) to plot