% Manually indicate horizon
if ~exist('p','var')
  disp('Initializing setup')
  p=struct;
  p.analysisparams=analysissetup();
  %ctype='av10115';
  ctype='av10115-half';
  ncamera=6;
  for i=1:ncamera
    p.camera(i)=setupcamera(ctype,i);
  end
  p.led=struct('id',num2cell(1:numled()));
  p.colors={[1 1 1], [1 0 0], [0 1 0], [0 0 1],[1 1 0],[1 0 1], [0 1 1]};
  p.layout=layoutpolygon(8,6,false);
end

% Set camera exposures to 80ms
%setupcameras(p,'exptime',80);
if ~isfield(p.camera(1),'horizon')
  p=locatehorizon(p);
end

% Configure horizon
p=horizonfn(p);

% Setup initial visibility
[vis,p]=getvisible(p,'init',true,'setleds',false,'usefrontend',false,'navg',4);
vv=getvisible(p,'stats',true,'setleds',false,'usefrontend',false);
plotvisible(p,vv);

% Add ray image to structure (rays from each camera to each LED) to speed up target blocking calculation (uses true coords)
if ~isfield(p,'rays')
  disp('Precomputing rays');
  p.rays=createrays(p);
end

fectl(p,'start');

