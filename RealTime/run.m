% Run the entire calibration/analysis chain
doplot=0;
dolevcheck=false;

if ~exist('p','var')
  disp('Initializing setup')
  p=struct;
  p.analysisparams=analysissetup();
  %ctype='av10115';
  ctype='av10115-half';
  ncamera=6;
  for i=1:ncamera
    h=getsubsysaddr(sprintf('CA%d',i));
    physid=sscanf(h,'%*d.%*d.%*d.%d')-70;
    p.camera(i)=setupcamera(ctype,i,physid);
  end
  p.led=struct('id',num2cell(1:numled()));
  p.colors={[1 1 1], [1 0 0], [0 1 0], [0 0 1],[1 1 0],[1 0 1], [0 1 1]};

  p.layout=layoutpolygon(8,ncamera,0);
  plotlayout(p.layout);
end

% Set camera exposures to defaults
daymode;


if ~isfield(p.camera(1),'pixcalib')
  disp('Pixel calibration');
  [p,pcim]=pixcalibrate(p);
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

% Refine position of cameras
% Need to physically block cameras for this...
% Instead use prior calibration
if length(p.camera)==5
  cposcalib=[ 1.0417   -2.2688
              -1.3200   -2.1270
              -2.5115   -0.0019
              -1.3293    2.1081
              1.0517    2.2646];
elseif length(p.camera)==6
  cposcalib=[      2.0838   -2.5066
   -0.0987   -3.2321
   -2.3154   -2.2883
   -2.4100    2.1878
   -0.1048    3.2607
    2.0975    2.5106];
else
  cposcalib=[];
end
if length(p.layout.cpos)==length(cposcalib) && max(max(abs(cposcalib-p.layout.cpos)))<0.15
  fprintf('Forcing camera positions to previously calibrated values\n')
  p.layout.cpos=cposcalib;
else
  %layout.cpos=locatecameras();
  fprintf('*** Should run locatecameras()\n');
end

% Use the layout and the pixcalibration to update the anglemaps and the camera cdir
p=updateanglemap(p);

% Could also adjust positions of LEDs based on pixel calibration, but this doesn't work yet
%adjpos(p)

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


