% mainloop - used for both replays and real time
% Inputs: recvis struct
%	  timedreplay - true to run replay at same speed as original
%	  plots - set of plots to do ('stats','hypo','ana','vis')
%function recvis=mainloop(recvis,timedreplay,plots)
global recvis
userecvis=~isempty(recvis.vis);

if ismember('hypo',plots)
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
    vis=getvisible(recvis.p,'setleds',false);
    %vis=getvisfrompipe(recvis.p,recvis.randseed);
    if samp==1
        recvis.vis=vis;
    else
        recvis.vis=[recvis.vis,vis];
    end
  end
  if ismember('vis',plots)
    plotvisible(recvis.p,vis);
  end
  % Check for bad data
  transitions=sum(abs(diff(vis.v'))==1);
  if any(transitions>20) && samp>=suppressuntil
    suppressuntil=samp+50;
    fprintf('Too many LED blockage edges at snapshot %d: %s (ignoring until samp %d)\n', samp, sprintf('%d ',transitions),suppressuntil);
  end

  % Analyze data to estimate position of targets using layout
  pflg=0;
  if ismember('ana',plots)
    pflg=1;
  end
  if ismember('rays',plots)
    pflg=2;
  end
  snap{samp}=analyze(recvis.p,vis.v,pflg);
  snap{samp}.when=vis.when;
  snap{samp}.samp=samp;
  if samp==1
    snap{1}=inittgthypo(snap{1});
  else
    snap{samp}=updatetgthypo(recvis.p.layout,snap{samp-1},snap{samp},samp);
  end

  if ~userecvis
    snap{samp}.whendone=now;
    if samp==1
      recvis.snap=snap{samp};
    else
      recvis.snap=[recvis.snap,snap{samp}];
    end
  end

  if ismember('hypo',plots) && ~isempty(snap{samp}.hypo)
    setfig('plothypo');
    subplot(221);
    foundk=zeros(length(snap{samp}.hypo),1);

    if samp>1
        [ids,pid,cid]=intersect([snap{samp-1}.hypo.id],[snap{samp}.hypo.id]);
        for j=1:length(ids)
            hj=snap{samp-1}.hypo(pid(j));
            hk=snap{samp}.hypo(cid(j)); 
            plot([hj.pos(1),hk.pos(1)],[hj.pos(2),hk.pos(2)],'Color',id2color(ids(j)));
        end

        % Exits
        [~,exid]=setdiff([snap{samp-1}.hypo.id],[snap{samp}.hypo.id]);
        for j=1:length(exid)
            hj=snap{samp-1}.hypo(exid(j));
            plot(hj.pos(1),hj.pos(2),'x','Color',id2color(hj.id));
        end

        % Entrances
        [~,enid]=setdiff([snap{samp}.hypo.id],[snap{samp-1}.hypo.id]);
        for j=1:length(enid)
            hj=snap{samp}.hypo(enid(j));
            plot(hj.pos(1),hj.pos(2),'o','Color',id2color(hj.id));
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
            plot([samp-1,samp],[hj.pos(1),hk.pos(1)],'Color',id2color(hj.id));
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
            plot([samp-1,samp],[hj.pos(2),hk.pos(2)],'Color',id2color(hj.id));
        end
    end
  end
  
  
  % Display coords
%  fprintf('%3d@%4.1f ',samp,elapsed);
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
    fprintf('\n');
  end
  
  if samp>1
      if maxok
        oscupdate(recvis.p,samp,snap{samp},snap{samp-1});
          maxok=updatemax(recvis.p,snap{samp},snap{samp-1});
          if ~maxok
              fprintf('Disabling MAX updates\n');
          end
      end
  else
    oscupdate(recvis.p,samp,snap{samp});
      maxok=updatemax(recvis.p,snap{samp});
      if ~maxok
          fprintf('Disabling MAX updates\n');
      end
  end

  if ~userecvis
    % updateleds(recvis.p,snap{samp});
    visleds(recvis.p,vis);
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
oscupdate(recvis.p,samp);

if ismember('stats',plots)
  plotstats(snap);
end