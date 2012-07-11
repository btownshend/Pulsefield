% ledspiral - compute position of LED's and cameras in spiral setup
% p - parameters
% doplot - 1 to plot
function layout=layoutspiral(p,doplot)
if nargin<2
  doplot=0;
end
% Work in meters
ledspacing=1/32;
leglen=(8*12)/39.37;
strutlength=leglen/2;
nlegs=8;
opening=1.0;   % 1 meter opening
maxtheta=2*pi*8/7.69;
r0=(leglen*nlegs)/maxtheta-opening/2*maxtheta/(2*pi);
theta=(0:12000*1.1)/12000*maxtheta;
% Correct for shorter segments
meanrot=maxtheta/nlegs;
r0=r0/(sin(meanrot/2)/(meanrot/2));
r=r0+theta/(2*pi)*opening;
[x,y]=pol2cart(theta,r);
pos=[x(1),y(1)];
for i=1:length(x)
  if norm([x(i),y(i)]-pos(end,:))>=leglen
    pos(end+1,:)=[x(i),y(i)];
  end
  if size(pos,1)>nlegs
    break;
  end
end
x=x(1:i); y=y(1:i);  % Cut off unneeded part
angles=atan2(pos(:,2),pos(:,1));
cornerangles=(pi-unwrap(diff(angles)));
disp(['Angles: ',sprintf('%.1f ',cornerangles*180/pi)]);

% LED strip position
ledcorners=pos(1,:);
for i=2:size(pos,1)-1
  frac=((strutlength/2)/sin(cornerangles(i)/2)) /leglen;
  ledcorners(end+1,:)=pos(i-1,:)*frac+pos(i,:)*(1-frac);
  ledcorners(end+1,:)=pos(i+1,:)*frac+pos(i,:)*(1-frac);
%  fprintf('Length of strut = %.1f\n',norm(ledcorners(end,:)-ledcorners(end-1,:)));
end
ledcorners(end+1,:)=pos(end,:);
dled=ledcorners(2:end,:)-ledcorners(1:end-1,:);
ledlength=sum(sqrt(dled(:,1).^2+dled(:,2).^2));
fprintf('Total LED length = %.1fm\n', ledlength);

% Camera positions
camera=[];
cdir=[];
for i=2:2:size(pos,1)
  camera(end+1,:)=pos(i,:);
  cdir(end+1,:)=-pos(i,:)/norm(pos(i,:));  % Directed towards center
end

% LED positions
seg=1;
offset=ledspacing/2;   % Initial LED
lpos=[];
ldir=[];
for seg=1:size(ledcorners,1)-1
  v=ledcorners(seg+1,:)-ledcorners(seg,:);
  seglen=norm(v);
  segpos=offset:ledspacing:seglen;
  v=v/seglen;
  ind=size(lpos,1);
  lpos(ind+1:ind+length(segpos),:)=[ledcorners(seg,1)+segpos*v(1);ledcorners(seg,2)+segpos*v(2)]';
  ldir(ind+1:ind+length(segpos),1)=v(2);
  ldir(ind+1:ind+length(segpos),2)=v(1);
  offset=segpos(end)+ledspacing-seglen;
end

if doplot
  setfig('layoutspiral');
  clf;
  plot(x,y,':');
  hold on;
  title('layoutspiral');
  h=plot(pos(:,1),pos(:,2),'r');
  labels={'Main struts'};
  for i=2:2:size(ledcorners,1)-1
    h(2)=plot(ledcorners([i,i+1],1),ledcorners([i,i+1],2),'g');
  end
  labels{2}='Secondary struts';
  % Cameras
  for i=1:size(camera,1)
    h(3)=plot(camera(i,1),camera(i,2),'ko');
  end
  labels{3}='Cameras';

  % Lines of sight
  h(4)=plot(pos([6,end],1),pos([6,end],2),'m');
  labels{4}='Sightlines';
  plot(pos([2,8],1),pos([2,8],2),'m');
  legend(h,labels);
  axis equal
  pause(0);
end

% Setup return variables
layout=struct('cpos',camera,'cdir',cdir,'lpos',lpos,'ldir',ldir,'active',ledcorners);
