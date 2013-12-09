% Run the entire calibration/analysis chain
doplot=3;
dolevcheck=false;

if ~exist('p','var')
  disp('Initializing setup')
  p=struct;
  p.analysisparams=analysissetup();
  p.analysisparams.detector=1;   % Use foreground/background detector
  p.analysisparams.mintgtdiam=0.08;   % Smaller targets (for feet) -- 8cm diameter
  p.analysisparams.maxfalsegapsep=0.5;   % For gap between feet
  %ctype='av10115';
  ctype='av10115-half';
  ncamera=6;
  for i=1:ncamera
    h=getsubsysaddr(sprintf('CA%d',i));
    physid=sscanf(h,'%*d.%*d.%*d.%d')-70;
    p.camera(i)=setupcamera(ctype,i,physid);
  end
  nled=600;
  p.led=struct('id',num2cell(1:nled));
  p.colors={[1 1 1], [1 0 0], [0 1 0], [0 0 1],[1 1 0],[1 0 1], [0 1 1]};

  % office
  %p.layout=layoutroom(130/39.37,145/39.37,'ncamera',ncamera,'nled',nled);
  % CCRMA stage  length(from camers 3,4 to screen)=284", width from windows to walls=277"
  p.layout=layoutroom(284/39.37,277/39.37,'ncamera',ncamera,'nled',nled);
  p.noleds=true;
  plotlayout(p.layout);
end
% Make sure front end is stopped
fectl(p,'quit');

% Set camera exposures to defaults
% Fix the exposure time to 40ms and maximum analog gain to 768 (12*64)
setupcameras(p,'mode','highspeed','exptime',80,'daynight','day','analoggain',12,'illum','automatic');   
pause(2);
% Turn off auto exposure
lockexposure(p,1);

if ~isfield(p.camera(1),'pixcalib')
  disp('Pixel calibration');
  p=pixcalibrate_noled(p,'doplot',doplot);
  if 0 && doplot
    for i=1:length(p.camera)
      plotpixcalib(p.camera(i));
    end
    plotdistortion(p);
  end
end
plotvalid(p);

if ~isfield(p.camera(1),'viscache')
  disp('Visible calibration');
  % Use a larger window to improve foreground object detection when background is bland
  [allvis,p]=getvisible(p,'init','wsize',[11,5],'usefrontend',false,'disableleds',false);
end

if 0 && ~isfield(p,'crosstalk')
  % Only really needed 
  disp('Crosstalk calculation');
  p=crosstalk(p);
end

% Add ray image to structure (rays from each camera to each LED) to speed up target blocking calculation (uses true coords)
if ~isfield(p,'rays')
  disp('Precomputing rays');
  p.rays=createrays(p);
end

% Start frontend and init
if ~fectl(p,'start') 
  error('Failed to start frontend\n');
end

disp('Measuring');
vis=getvisible(p,'stats',true);
if doplot>1
  plotvisible(p,vis);
end

% Analyze data to estimate position of targets using layout
samp=analyze(p,vis.v,doplot);

% Run recording
%recvis=recordvis(p,5);
%recanalyze(recvis); % Analyze whole recording to get tracking
%recanalyze(recvis,2);   % Analyze specific sample

if dolevcheck
  disp('Check levels');
  levcheck(p,1);
end

save('p.mat','p');


