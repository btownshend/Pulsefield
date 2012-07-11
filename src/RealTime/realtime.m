doplot=0;
if ~exist('userecvis')
  userecvis=0;
end
if ~userecvis
  fprintf('Type saverecvis when done to save\n');
end
if userecvis && ~exist('timedreplay')
  timedreplay=0;   % Set to 1 before running script to replay at same pacing as recording
end
s1=arduino_ip(1);
if ~userecvis
  recvis=struct('p',p,'layout',layout,'rays',rays,'vis',[],'tgtestimate',[],'possible',{{}});
  setled(s1,[0,numled()-1],p.colors{1},1); show(s1); sync(s1);
end
plotlayout(layout)
hold on;
samp=1;
tlen=100;
tgtestimate={};
possible={};
starttime=now;
while ~userecvis || samp<=length(recvis.vis)
  tic;
    
  if userecvis
    vis=recvis.vis(samp);
    if timedreplay
      sleeptime=((vis.when-recvis.vis(1).when)-(now-starttime))*24*3600;
      if sleeptime>0
        pause(sleeptime);
      end
    end
  else
    vis=getvisible(p,0);
    recvis.vis=[recvis.vis,vis];
  end
  if doplot>1
    plotvisible(p,vis);
  end
  % Analyze data to estimate position of targets using layout
  [possible{samp},tgtestimate{samp}]=analyze(p,layout,vis.v,rays,doplot);
  if ~userecvis
    recvis.tgtestimate=[recvis.tgtestimate,tgtestimate{samp}];
    recvis.possible={recvis.possible,possible{samp}};
  end
  tgtestimate{samp}.when=vis.when;
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
  updatemax(tgtestimate{samp}.tpos);
  updateleds(p,layout,tgtestimate{samp}.tpos);
  loopend=toc;
%  fprintf('Loop time = %.2f seconds\n', loopend);
  pause(0.01);   % Give time for figures to refresh
  samp=samp+1;
end
% Turn off LEDs
setled(s1,-1,[0,0,0],1);show(s1);
