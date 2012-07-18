plothypo=1;
plottgts=0;
plotvis=0;
plotana=0;
plotstats=1;
if ~exist('userecvis')
  userecvis=0;
end
if ~userecvis
  fprintf('Type saverecvis when done to save\n');
end
if userecvis && ~exist('timedreplay')
  timedreplay=0;   % Set to 1 before running script to replay at same pacing as recording
end
if ~userecvis
  s1=arduino_ip(1);
  recvis=struct('p',p,'layout',layout,'rays',rays,'vis',[],'tgtestimate',[],'possible',{{}});
  setled(s1,[0,numled()-1],p.colors{1},1); show(s1); sync(s1);
end
if plottgts
  setfig('plottgts');clf;
  plotlayout(recvis.layout,0)
  hold on;
end
if plothypo
  setfig('plothypo');clf;
  plotlayout(recvis.layout,0)
  hold on;
end
samp=1;
tlen=100;
tgtestimate={};
hypo={};
possible={};
starttime=now;
while ~userecvis || samp<=length(recvis.vis)
  tic;
    
  elapsed=(now-starttime)*24*3600;
  if userecvis
    vis=recvis.vis(samp);
    if timedreplay
      sleeptime=((vis.when-recvis.vis(1).when)-(now-starttime))*24*3600;
      if sleeptime>0
        pause(sleeptime);
      end
    end
  else
    vis=getvisible(recvis.p,0);
    recvis.vis=[recvis.vis,vis];
  end
  if plotvis
    plotvisible(recvis.p,vis);
  end
  % Analyze data to estimate position of targets using layout
  [possible{samp},tgtestimate{samp}]=analyze(recvis.p,recvis.layout,vis.v,recvis.rays,plotana);
  if ~userecvis
    recvis.tgtestimate=[recvis.tgtestimate,tgtestimate{samp}];
    recvis.possible={recvis.possible,possible{samp}};
  end
  tgtestimate{samp}.when=vis.when;
  if samp==1
    % hypo{samp}=inithypo(tgtestimate{1},recvis.rays.imap);
    hypo{samp}=inittgthypo(tgtestimate{1});
  else
    dt=(recvis.vis(samp).when-recvis.vis(samp-1).when)*24*3600;
    maxspeed=1.3;  % m/s
    maxmovement=maxspeed*dt;
    %    hypo{samp}=updatehypo(recvis.p,recvis.layout,recvis.rays.imap,hypo{samp-1},possible{samp},maxmovement);
    hypo{samp}=updatetgthypo(recvis.p,recvis.layout,recvis.rays.imap,hypo{samp-1},tgtestimate{samp},maxmovement);
  end
  
  if plottgts
    setfig('plottgts');
    % Erase old symbols
    if samp>1 & size(tgtestimate{samp-1}.tpos,1)>0
      plot(tgtestimate{samp-1}.tpos(:,1),tgtestimate{samp-1}.tpos(:,2),'+w');
    end

    if size(tgtestimate{samp}.tpos,1)>0
      plot(tgtestimate{samp}.tpos(:,1),tgtestimate{samp}.tpos(:,2),'+r');
      if samp>1
        for j=1:size(tgtestimate{samp}.tpos,1)
          tj=tgtestimate{samp}.tpos(j,:);
          for k=1:size(tgtestimate{samp-1}.tpos,1)
            tk=tgtestimate{samp-1}.tpos(k,:);
            if norm(tj-tk)<0.3
              % Join nearby positions
              plot([tj(1),tk(1)],[tj(2),tk(2)],'g');
            end
          end
        end
      end
    end
  end
  
  if plothypo
    setfig('plothypo');
    cols='rgbcymk';
    foundk=zeros(length(hypo{samp}),1);

    if samp>1
      foundj=zeros(length(hypo{samp-1}),1);
      for j=1:length(hypo{samp-1})
        hj=hypo{samp-1}(j);
        for k=1:length(hypo{samp})
          hk=hypo{samp}(k);
          if hj.id==hk.id
            % Join nearby positions
            col=cols(mod(hj.id-1,length(cols))+1);
            plot([hj.pos(1),hk.pos(1)],[hj.pos(2),hk.pos(2)],col);
            foundj(j)=1;
            foundk(k)=1;
          end
        end
      end
      for j=1:length(hypo{samp-1})
        if ~foundj(j)
          hj=hypo{samp-1}(j);
          col=cols(mod(hj.id-1,length(cols))+1);
          plot(hj.pos(1),hj.pos(2),['x',col]);
        end
      end
    end
    for k=1:length(hypo{samp})
      if ~foundk(k)
        hk=hypo{samp}(k);
        col=cols(mod(hk.id-1,length(cols))+1);
        plot(hk.pos(1),hk.pos(2),['o',col]);
      end
    end
  end

  % Display coords
  fprintf('%3d@%4.1f ',samp,elapsed);
  for j=1:size(tgtestimate{samp}.tpos,1)
    tj=tgtestimate{samp}.tpos(j,:);
    fprintf('T%d:(%.2f,%.2f) ',j,tj);
  end
  for j=1:length(hypo{samp})
    hj=hypo{samp}(j);
    if samp>1 && length(hypo{samp-1})>=j && hypo{samp-1}(j).id==hj.id
      hk=hypo{samp-1}(j);
      spd=norm(hj.pos-hk.pos)/dt;
    else
      spd=nan;
    end
    fprintf('H%d:(%.2f,%.2f)@%.1f m/s ',hj.id,hj.pos,spd);
  end
  fprintf('\n');
  
  if samp>1
    updatemax(tgtestimate{samp}.tpos,tgtestimate{samp-1}.tpos);
  else
    updatemax(tgtestimate{samp}.tpos);
  end
  if ~userecvis
    updateleds(recvis.p,recvis.layout,tgtestimate{samp}.tpos);
  end
  loopend=toc;
%  fprintf('Loop time = %.2f seconds\n', loopend);
  pause(0.01);   % Give time for figures to refresh
  samp=samp+1;
end
% Turn off any remaining notes
updatemax([]);

if ~userecvis
  % Turn off LEDs
  setled(s1,-1,[0,0,0],1);show(s1);
end

if plotstats
  setfig('hypo stats');clf;
  time=[];speed=[];area=[];
  for i=1:length(hypo)
    for j=1:length(hypo{i})
      speed(i,j)=hypo{i}(j).speed;
      area(i,j)=hypo{i}(j).area;
      time(i,j)=hypo{i}(j).lasttime;
      majoraxislength(i,j)=hypo{i}(j).majoraxislength;
      minoraxislength(i,j)=hypo{i}(j).minoraxislength;
    end
  end
  speed(speed==0)=nan;
  area(area==0)=nan;
  majoraxislength(majoraxislength==0)=nan;
  minoraxislength(minoraxislength==0)=nan;
  time=(time-min(time(time>0)))*24*3600;
  time(time<0)=nan;
  subplot(411);
  plot(time,speed);
  xlabel('Time (sec)');
  ylabel('Speed (m/s)');
  title('Speed');
  c=axis;c(3)=0;c(4)=3;axis(c);
  subplot(412);
  plot(time,area);
  xlabel('Time (sec)');
  ylabel('Area (m2)');
  title('Area');
  c=axis;c(3)=0;c(4)=0.25;axis(c);
  subplot(413);
  plot(time,majoraxislength);
  xlabel('Time (sec)');
  ylabel('Length (m)');
  title('Major Axis Length');
  c=axis;c(3)=0;c(4)=0.8;axis(c);
  subplot(414);
  plot(time,minoraxislength);
  xlabel('Time (sec)');
  ylabel('Length (m)');
  title('Minor Axis Length');
  c=axis;c(3)=0;c(4)=0.8;axis(c);
end
  
  