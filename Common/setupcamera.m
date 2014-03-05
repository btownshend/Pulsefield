% setup - setup all parameters
function cam=setupcamera(type,id,physid)
cam.id=id;
cam.type=type;
cam.fisheye=0;
if strcmp(cam.type,'guppy')
  % Guppy F-503*
  cam.sensorwidth=5.7; % Sensor width (mm) 
  cam.hpixels=2592;
  cam.lens=3.5;	% Cam.Lens FL (mm)
elseif strcmp(cam.type,'manta')
  % Manta G-125
  cam.sensorwidth=4.8; % Sensor width (mm)
  cam.hpixels=1280;
  cam.lens=3.5;	% Cam.Lens FL (mm)
elseif strcmp(cam.type,'ptz212')
  % Axis 212 PTZ
  cam.hpixels=2048;
  cam.vpixels=1536;
  cam.sensorwidth=14.8; % Sensor width (mm)
  cam.lens=2.7;	% Cam.Lens FL (mm)
  cam.fisheye=1;
  cam.fov=140*pi/180;
elseif strcmp(cam.type,'fe8171')
  % Vivotek FE8171
  cam.hpixels=2048;
  cam.vpixels=1536;
  htov=cam.hpixels/sqrt(cam.vpixels^2+cam.hpixels^2);
  cam.sensorwidth=1/2*25.4*htov; % Sensor width (mm)
  cam.fisheye=1;   % Guessing that it is a cam.fisheye cam.lens
  cam.fov=pi;
elseif strcmp(cam.type,'fe8362')
  % Vivotek FE8362
  cam.hpixels=1920;
  cam.vpixels=1536;
  htov=cam.hpixels/sqrt(cam.vpixels^2+cam.hpixels^2);
  cam.sensorwidth=1/2.7*25.4*htov; % Sensor width (mm)
  cam.fisheye=0;
  cam.fov=119*pi/180;   % Assuming mounted with diagonal view to get DFOV
elseif strcmp(cam.type,'av10115')
  % Arecont Vision AV10115 with SUNEX 2.67mm fisheye
  cam.hpixels=3648;
  cam.vpixels=2752;
  cam.sensorwidth=1.67*cam.hpixels/1000; % Sensor width (mm)
  cam.fisheye=1;
  cam.fov=145*pi/180;
elseif strcmp(cam.type,'av10115-4mm')
  % Arecont Vision AV10115 with Arecont 4mm 
  cam.hpixels=3648;
  cam.vpixels=2752;
  cam.sensorwidth=1.67*cam.hpixels/1000; % Sensor width (mm)
  cam.fisheye=0;
  cam.lens=4.0;
elseif strcmp(cam.type,'av10115-half')
  % Arecont Vision AV10115 with SUNEX 2.67mm fisheye (half resolution mode)
  cam.hpixels=3648/2;
  cam.vpixels=2752/2;
  cam.sensorwidth=1.67*cam.hpixels/1000; % Sensor width (mm)
  cam.fisheye=1;
  cam.fov=145*pi/180;
else
  error('Unknown camera type: "%s"',cam.type);
end

if ~isfield(cam,'fov')
  cam.fov=2*atan(cam.sensorwidth/(2*cam.lens));
end
  
fprintf('FOV=%.1f degrees\n',cam.fov*180/pi);

% Sensor angle map (converts sensor position in pixels to angle in radians)
if cam.fisheye
  cam.anglemap=(((1:cam.hpixels)-0.5)/cam.hpixels-0.5)*cam.fov;
else
  cam.anglemap=atan(2*(((1:cam.hpixels)-0.5)/cam.hpixels-0.5))*cam.fov*(4/pi)/2;
end

if nargin>=3
  cam.physid=id;
end
