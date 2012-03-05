% Parameters
p.isize=500;
p.sidelength=5;	% 5m sides
p.scale=p.isize/p.sidelength;
p.sidelength=p.sidelength*p.scale;
p.ledspacing=1/32*p.scale;	% 32 LED/m
p.ncameras=4;
p.cextend=0*p.scale;	% Extend camera this distance outside area (possibly using mirrors)
p.virtualcameras=0;
p.cam.name='fe8171'
p.cam.fisheye=0;
if strcmp(p.cam.name,'guppy')
  % Guppy F-503
  p.cam.sensorwidth=5.7; % Sensor width (mm) 
  p.cam.hpixels=2592;
  p.cam.lens=3.5;	% P.Cam.Lens FL (mm)
elseif strcmp(p.cam.name,'manta')
  % Manta G-125
  p.cam.sensorwidth=4.8; % Sensor width (mm)
  p.cam.hpixels=1280;
  p.cam.lens=3.5;	% P.Cam.Lens FL (mm)
elseif strcmp(p.cam.name,'ptz212')
  % Axis 212 PTZ
  p.cam.hpixels=2048;
  p.cam.vpixels=1536;
  htov=p.cam.hpixels/sqrt(p.cam.vpixels^2+p.cam.hpixels^2);
  p.cam.sensorwidth=14.8; % Sensor width (mm)
  p.cam.lens=2.7;	% P.Cam.Lens FL (mm)
  p.cam.fisheye=1;
  p.cam.fov=140*pi/180;
elseif strcmp(p.cam.name,'fe8171')
  % Vivotek FE8171
  p.cam.hpixels=2048;
  p.cam.vpixels=1536;
  htov=p.cam.hpixels/sqrt(p.cam.vpixels^2+p.cam.hpixels^2);
  p.cam.sensorwidth=1/2*25.4*htov; % Sensor width (mm)
  p.cam.fisheye=1;   % Guessing that it is a p.cam.fisheye p.cam.lens
  p.cam.fov=pi;
else
  error('Unknown camera type: "%s"',p.cam.name);
end

if p.cam.fisheye==0
  p.cam.fov=2*atan(p.cam.sensorwidth/(2*p.cam.lens));
end
  
fprintf('FOV=%.1f degrees\n',p.cam.fov*180/pi);
if p.cextend>0
  fprintf('Mirror width=%.1f cm\n', p.cextend*p.cam.fov/p.scale*100);
end
if p.cextend>0 && p.virtualcameras==1
  % Assume camera is looking down at two mirrors at the corner that have different yaw giving
  % 2 separate views
  % Calculate position of those 2 p.virtual cameras
  virtsep=p.cextend/2;	% Separation of p.virtual cameras
  offa=pi/4+virtsep/p.cextend;
  offset(1,:)=[cos(offa),sin(offa)]*p.cextend;
  offset(2,:)=[sin(offa),cos(offa)]*p.cextend;
  cpos=[];cdir=[];
  cpos(1,:)=[0,0]+offset(1,:).*[-1,-1];
  cdir(1,:)=[0,0]-cpos(end,:);
  cpos(end+1,:)=[0,0]+offset(2,:).*[-1,-1];
  cdir(end+1,:)=[0,0]-cpos(end,:);
  cpos(end+1,:)=[p.sidelength,0]+offset(1,:).*[1,-1];
  cdir(end+1,:)=[p.sidelength,0]-cpos(end,:);
  cpos(end+1,:)=[p.sidelength,0]+offset(2,:).*[1,-1];
  cdir(end+1,:)=[p.sidelength,0]-cpos(end,:);
  cpos(end+1,:)=[p.sidelength,p.sidelength]+offset(1,:).*[1,1];
  cdir(end+1,:)=[p.sidelength,p.sidelength]-cpos(end,:);
  cpos(end+1,:)=[p.sidelength,p.sidelength]+offset(2,:).*[1,1];
  cdir(end+1,:)=[p.sidelength,p.sidelength]-cpos(end,:);
  cpos(end+1,:)=[0,p.sidelength]+offset(1,:).*[-1,1];
  cdir(end+1,:)=[0,p.sidelength]-cpos(end,:);
  cpos(end+1,:)=[0,p.sidelength]+offset(2,:).*[-1,1];
  cdir(end+1,:)=[0,p.sidelength]-cpos(end,:);
elseif p.cextend>0 && p.virtualcameras==2
  % Assume camera is looking down at three mirrors on the sides that have different yaw giving
  % 3 separate views.  Calculate position of these p.virtual cameras
  virtsep=p.cextend/2;	% Separation of p.virtual cameras
  offa=(pi-p.cam.fov)/2-p.cextend/p.sidelength;
  locs=[0.5 0
        0 0.5
        1 0.5
        0.5 1
       ] * p.sidelength;
  cpos=[];cdir=[];
  for i=1:size(locs,1)
    cdir(end+1,:)=[0.5,0.5]*p.sidelength-locs(i,:); cdir(end,:)=cdir(end,:)/norm(cdir(end,:));
    cpos(end+1,:)=locs(i,:)-cdir(end,:)*p.cextend;
    cdir(end+1,:)=cdir(end,:)*[cos(offa) sin(offa); -sin(offa) cos(offa)];
    cpos(end+1,:)=locs(i,:)-cdir(end,:)*p.cextend;
    cdir(end+1,:)=cdir(end-1,:)*[cos(offa) -sin(offa); sin(offa) cos(offa)];
    cpos(end+1,:)=locs(i,:)-cdir(end,:)*p.cextend;
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
       ] * p.sidelength;
  cpos=cpos(1:p.ncameras,:);
  cdir=[];
  for i=1:size(cpos,1)
    cdir(i,:)=[0.5,0.5]*p.sidelength-cpos(i,:);
    cdir(i,:)=cdir(i,:)/norm(cdir(i,:));
  end
  cpos=cpos-cdir*p.cextend;
end

% Normalize all the direction vectors
for i=1:size(cdir,1)
  cdir(i,:)=cdir(i,:)/norm(cdir(i,:));
end

% Led positions
nledperside=floor(p.sidelength/p.ledspacing);
for i=1:nledperside
  pv=(i-0.5)*p.ledspacing;
  lpos(i,:)=[0,pv];
  lpos(1*nledperside+i,:)=[pv,p.sidelength];
  lpos(3*nledperside+1-i,:)=[p.sidelength,pv];
  lpos(4*nledperside+1-i,:)=[pv,0];
end

% Sensor angle map (converts sensor position in pixels to angle in radians)
if p.cam.fisheye
  p.cam.anglemap=(((1:p.cam.hpixels)-0.5)/p.cam.hpixels-0.5)*p.cam.fov;
else
  p.cam.anglemap=atan(2*(((1:p.cam.hpixels)-0.5)/p.cam.hpixels-0.5))*p.cam.fov*(4/pi)/2;
end
