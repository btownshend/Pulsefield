% layoutlinear - linear strip of LEDs
% p - parameters
% nled - number of leds
% cdist - distance in meters from middle camera to middle led
function layout=layoutlinear(p,nled)
% Work in meters
ledspacing=1/32;
firstled=.085;
firstcam=.395;
cspacing=0.46;
cdist=3.93;

% LED positions
for i=1:nled
  lpos(i,:)=[firstled+ledspacing*(i-1),cdist];
  ldir(i,:)=[0,-1];
end

% Camera positions
cpos=[];
cdir=[];
ncamera=length(p.camera)
for i=1:ncamera
  cpos(i,:)=[firstcam+cspacing*(i-1),0];
  cdir(i,:)=[0,1];
end
  
% Active region
active=[lpos(1,:)
        lpos(end,:)
        cpos(end,:)+[0 0.5]
        cpos(1,:)+[0 0.5]];

% Setup return variables
layout=struct('cpos',cpos,'cdir',cdir,'lpos',lpos,'ldir',ldir,'active',active);
