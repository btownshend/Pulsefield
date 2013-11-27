% layoutroom - compute position of (virtual) LED's and cameras in a room with usable walls on 3 sides
% ncameras - number of cameras
% width - width of room in m(open side)
% length - length of room
% nled - number of (virtual) LEDs along walls of room
% doplot - 1 to plot
%
% X will run down length of room, Y across
% Y-axis is entry area
function layout=layoutroom(width,length,ncameras,nled,doplot)
if nargin<5
  doplot=0;
end


% Work in meters
inchpermeter=39.3700787;
feetpermeter=inchpermeter/12;

% Camera positions
pairsep=(27+3/32)/inchpermeter;
cpos=[];
cdir=[];
% Arranged clockwise
cpos(1,:)=[pairsep,0]/sqrt(2);
cpos(2,:)=[0,pairsep]/sqrt(2);
cpos(3:4,1)=0;
cpos(3,2)=width/2-pairsep/2;
cpos(4,2)=width/2+pairsep/2;
cpos(5,:)=[0,width-pairsep/sqrt(2)];
cpos(6,:)=[pairsep/sqrt(2),width];
cdir(1:2,1:2)=1/sqrt(2);
cdir(3:4,1)=1;
cdir(3:4,2)=0;
cdir(5:6,1)=1/sqrt(2);
cdir(5:6,2)=-1/sqrt(2);
if ncameras==4
  cpos=cpos([1,2,5,6],:);
  cdir=cdir([1,2,5,6],:);
elseif ncameras==2
  cpos=cpos([3,4],:);
  cdir=cdir([3,4],:);
else
  error('layoutroom only implemented for 2,4, or 6 cameras');
end
croll(1:ncameras)=0;   % Roll of cameras in radian around axis in cdir using right-hand rule
cposz(1:ncameras)=0.047;
cdirz(1:ncameras)=0;

% Virtual LED positions
equalangle=0;
if equalangle
  % Points along wall uniformly spaced in angle from the point (0,width/2)
  angle=-pi/2:pi/(nled-1):pi/2;
  far=cos(angle)/length > abs(sin(angle)/(width/2));   % Rays that will strike far wall
  top=~far & angle<0;
  bottom=~far & angle>0;
  lpos(far,1)=length;
  lpos(far,2)=-tan(angle(far))*length+width/2;
  lpos(top,1)=(-width/2) ./tan(angle(top));
  lpos(top,2)=width;
  lpos(bottom,1)=(width/2) ./tan(angle(bottom));
  lpos(bottom,2)=0;
else
  % Equal spacing
  perimeter=2*length+width;
  spacing=perimeter/(nled-1);
  nside=round(length/perimeter*nled);
  nfar=nled-2*nside;
  top=1:nside;
  far=nside+(1:nfar);
  bottom=nside+nfar+(1:nside);
  lpos(top,1)=(0:(nside-1))*spacing;
  lpos(top,2)=width;
  lpos(far,1)=length;
  lpos(far,2)=width/2-((0:(nfar-1))-(nfar-1)/2)*spacing;
  lpos(bottom,1)=((nside-1):-1:0)*spacing;
  lpos(bottom,2)=0;
end

ldir(far,1)=-1; ldir(far,2)=0;
ldir(bottom|top,1)=0; ldir(bottom,2)=1; ldir(top,2)=-1;

% Active area
active=[0,0
        length,0
        length,width
        0,width
        0,0];
pos=active;

% Entry is in middle of opening
entrywidth=0.5;
entry=[0 width/2];

% Flag LEDs outside of active region (so we don't assume that blockage of them is due to something inside active region)
outsider=false(1,nled); % None

% Setup return variables
layout=struct('cpos',cpos,'cdir',cdir,'croll',croll,'cposz',cposz,'cdirz',cdirz,'lpos',lpos,'ldir',ldir,'active',active,'pos',pos,'entry',entry,'outsider',outsider);
