% layout - setup desired positions of cameras, leds
function layout=layoutsquare(p,doplot)
if nargin<2
  doplot=0;
end
if length(p.camera)~=4
  error('Expected length(p.camera) to be 4');
end

cextend=0;	% Extend camera this distance outside area (possibly using mirrors)
virtualcameras=0;
ncamera=4;
sidelength=5;	% 5m sides
ledspacing=1/32;	% 32 LED/m

if cextend>0
  fprintf('Mirror width=%.1f cm\n', cextend*p.camera(1).fov*100);
end
if cextend>0 && virtualcameras==1
  % Assume camera is looking down at two mirrors at the corner that have different yaw giving
  % 2 separate views
  % Calculate position of those 2 virtual cameras
  virtsep=cextend/2;	% Separation of virtual cameras
  offa=pi/4+virtsep/cextend;
  offset(1,:)=[cos(offa),sin(offa)]*cextend;
  offset(2,:)=[sin(offa),cos(offa)]*cextend;
  cpos=[];cdir=[];
  cpos(1,:)=[0,0]+offset(1,:).*[-1,-1];
  cdir(1,:)=[0,0]-cpos(end,:);
  cpos(end+1,:)=[0,0]+offset(2,:).*[-1,-1];
  cdir(end+1,:)=[0,0]-cpos(end,:);
  cpos(end+1,:)=[sidelength,0]+offset(1,:).*[1,-1];
  cdir(end+1,:)=[sidelength,0]-cpos(end,:);
  cpos(end+1,:)=[sidelength,0]+offset(2,:).*[1,-1];
  cdir(end+1,:)=[sidelength,0]-cpos(end,:);
  cpos(end+1,:)=[sidelength,sidelength]+offset(1,:).*[1,1];
  cdir(end+1,:)=[sidelength,sidelength]-cpos(end,:);
  cpos(end+1,:)=[sidelength,sidelength]+offset(2,:).*[1,1];
  cdir(end+1,:)=[sidelength,sidelength]-cpos(end,:);
  cpos(end+1,:)=[0,sidelength]+offset(1,:).*[-1,1];
  cdir(end+1,:)=[0,sidelength]-cpos(end,:);
  cpos(end+1,:)=[0,sidelength]+offset(2,:).*[-1,1];
  cdir(end+1,:)=[0,sidelength]-cpos(end,:);
elseif cextend>0 && virtualcameras==2
  % Assume camera is looking down at three mirrors on the sides that have different yaw giving
  % 3 separate views.  Calculate position of these virtual cameras
  virtsep=cextend/2;	% Separation of virtual cameras
  offa=(pi-p.camera(1).fov)/2-cextend/sidelength;
  locs=[0.5 0
        0 0.5
        1 0.5
        0.5 1
       ] * sidelength;
  cpos=[];cdir=[];
  for i=1:size(locs,1)
    cdir(end+1,:)=[0.5,0.5]*sidelength-locs(i,:); cdir(end,:)=cdir(end,:)/norm(cdir(end,:));
    cpos(end+1,:)=locs(i,:)-cdir(end,:)*cextend;
    cdir(end+1,:)=cdir(end,:)*[cos(offa) sin(offa); -sin(offa) cos(offa)];
    cpos(end+1,:)=locs(i,:)-cdir(end,:)*cextend;
    cdir(end+1,:)=cdir(end-1,:)*[cos(offa) -sin(offa); sin(offa) cos(offa)];
    cpos(end+1,:)=locs(i,:)-cdir(end,:)*cextend;
  end
else
  % 4 cameras on corners
  cpos=[
        0.5 0
        0 0.5
        1 0.5
        0.5 1
        0 0
        0 1
        1 0
        1 1
       ] * sidelength;
  cpos=cpos(1:ncamera,:);
  cdir=[];
  for i=1:size(cpos,1)
    cdir(i,:)=[0.5,0.5]*sidelength-cpos(i,:);
    cdir(i,:)=cdir(i,:)/norm(cdir(i,:));
  end
  cpos=cpos-cdir*cextend;
end

% Normalize all the direction vectors
for i=1:size(cdir,1)
  cdir(i,:)=cdir(i,:)/norm(cdir(i,:));
end

% Led positions
nledperside=floor(sidelength/ledspacing);
for i=1:nledperside
  pv=(i-0.5)*ledspacing;
  lpos(i,:)=[0,pv];
  ldir(i,:)=[1,0];
  lpos(1*nledperside+i,:)=[pv,sidelength];
  ldir(1*nledperside+i,:)=[0,-1];
  lpos(3*nledperside+1-i,:)=[sidelength,pv];
  ldir(3*nledperside+1-i,:)=[-1,0];
  lpos(4*nledperside+1-i,:)=[pv,0];
  ldir(4*nledperside+1-i,:)=[0,1];
end

% Active area (where people could be) - define as a closed polygon using given vertices
active=[ 0 0
         0 1
         1 1
         1 0] * sidelength;
layout=struct('cpos',cpos,'cdir',cdir,'lpos',lpos,'ldir',ldir,'cextend',cextend,'active',active);
