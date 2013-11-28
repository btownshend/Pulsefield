% Run the entire calibration/analysis chain
doplot=0;
dolevcheck=false;

if ~exist('p','var')
  disp('Initializing setup')
  p=struct;
  p.analysisparams=analysissetup();
  %ctype='av10115';
  ctype='av10115-half';
  ncamera=2;
  for i=1:ncamera
    h=getsubsysaddr(sprintf('CA%d',i));
    physid=sscanf(h,'%*d.%*d.%*d.%d')-70;
    p.camera(i)=setupcamera(ctype,i,physid);
  end
  nled=10;
  p.led=struct('id',num2cell(1:nled));
  p.colors={[1 1 1], [1 0 0], [0 1 0], [0 0 1],[1 1 0],[1 0 1], [0 1 1]};

  p.layout=layoutroom(7,14,ncamera,nled);
  p.noleds=true;
  plotlayout(p.layout);
end

% Set camera exposures to defaults
setupcameras(p,'mode','quality','daynight','day','analoggain',25,'illum','indoor');

if ~isfield(p.camera(1),'pixcalib')
  disp('Pixel calibration');
  p=pixcalibrate_noled(p);
  if doplot
    for i=1:length(p.camera)
      plotpixcalib(p.camera(i));
    end
    plotdistortion(p);
  end
end
plotvalid(p);

if ~isfield(p.camera(1),'viscache')
  disp('Visible calibration');
  [allvis,p]=getvisible(p,'init',true,'usefrontend',false);
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


