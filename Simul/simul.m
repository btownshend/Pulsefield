% Simulate detection of blockage of a line array
addpath ../Common
doplot=1;
ncamera=4;

if exist('p')==0
  p=struct;
  p.analysisparams=analysissetup();
  for c=1:ncamera
    p.camera(c)=setupcamera('av10115',c);
  end
  p.simul=simulsetup();

  %  p.layout=layoutsquare(p, doplot);
  %  p.layout=layoutspiral(p,doplot);
  p.layout=layoutpolygon(6,ncamera,numled(),doplot);
  % p.layout=layoutlinear(p,100,doplot);

  % Add ray image to structure (rays from each camera to each LED) to speed up target blocking calculation (uses true coords)
  p.rays=createrays(p);

  % Run calibration.  First, calculate 'spos', the position on the sensors of each LED using design coords of everything.
  spos=calcspos(p);
  
  % Use the spos data to compute the estimated positions of LEDs and cameras, and camera gaze (cdir)
  calib=calibrate(p,spos,doplot);
end

% Simulate some targets
tgts={};v={}; tgtestimate={}; hypo={};
tgts{1}=settargets(p,5);
% Initial make it exactly correct
hypo{1}=inithypo(tgts{1},p.rays.imap);

nmoves=100;
dt=1;   % 1 second of target movement per update
for r=2:nmoves
  % Update actual positions
  maxchange=0.1; 	   % Maximum change in position in m (normal distn with stdev=maxchange/2, mean 0)
  tgts{r}=updatetargets(p,tgts{r-1});

  % Find visibility of each LED using true positions of everything (but using rays.raylines as a helper)
  v{r}=calcvisible(p,tgts{r},doplot);

  % Analyze data to estimate position of targets using calibrated layout
  [possible{r},tgtestimate{r}]=analyze(p,v{r},doplot,tgts{r});

  % Update hypotheses of positions
  hypo{r}=updatehypo(p,p.rays.imap,hypo{r-1},possible{r},maxchange);

  printtgtstats(p,tgts{r}.tpos,hypo{r}.pos,0.2);
  plottrack(tgts,hypo);
  pause(0);
end
