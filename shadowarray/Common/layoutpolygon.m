% layoutoctagon - compute position of LED's and cameras in octagon setup (with 1m entry opening)
% nlegs - number of sides of polygon
% doplot - 1 to plot
function layout=layoutpolygon(nlegs,ncameras,nleds,doplot)
if nargin<4
  doplot=0;
end
% Work in meters
if nargin<3 || isempty(nleds)
  ledspacing=1/32;
else
  ledspacing=nan;
end
inchpermeter=39.3700787;
feetpermeter=inchpermeter/12;
conlen=(1+1/16)/inchpermeter;   % Length that connectors add to legs
leglen=96.5/inchpermeter+2*conlen;
strutlength=4/feetpermeter;
entrylength=4/feetpermeter+2*conlen;
opening=2/feetpermeter;   % 24" opening

% Angle subtended by each 8' leg
legtheta=2*pi*(leglen*nlegs)/(leglen*nlegs+opening)/nlegs;
cornerangles=(pi-legtheta);
coninnerlen=conlen-(0.5/inchpermeter)/tan(cornerangles/2);
alpha=(pi-cornerangles)/2;
fprintf('Angle at main legs = %.1f degrees (rise/8=%.2f)\n', cornerangles*180/pi,tan(legtheta)*8);
fprintf('Angle at struts = %.1f degrees (rise/8=%.2f)\n', (pi-legtheta/2)*180/pi,tan(legtheta/2)*8);

% distance from center to leg intersections
r=leglen/2/cos(cornerangles/2);

% Distance of strut from corner along inside of aluminum leg
rstrutinside=strutlength/2/cos(alpha)-coninnerlen;
fprintf('Distance from end of main leg aluminum to end of struts along inside = %.1f cm (%.2f inches)\n', rstrutinside*100,rstrutinside*inchpermeter);

for i=1:nlegs+1
  [pos(i+1,1),pos(i+1,2)]=pol2cart(-((pi-cornerangles)*(i-1)+(2*pi-legtheta*nlegs)/2),r);
end
pos(1,1)=pos(2,1)+entrylength; pos(1,2)=pos(2,2);
pos(end+1,1)=pos(end,1)+entrylength; pos(end,2)=pos(end-1,2);

% Walkway angle
vwalk=2;
v1=pos(vwalk-1,:)-pos(vwalk,:);
v2=pos(vwalk+1,:)-pos(vwalk,:);
walkangle=acos(dot(v1,v2)/(norm(v1)*norm(v2)));
fprintf('Walkway angle (on outside) = %.1f degrees (rise/4=%.2f)\n', walkangle*180/pi, tan(pi-walkangle)*4);
walkstrutinside=strutlength/2/cos((pi-walkangle)/2)-coninnerlen;
fprintf('Distance from end of main leg aluminum to end of walkway struts along inside = %.1f cm (%.2f inches)\n', walkstrutinside*100,walkstrutinside*inchpermeter);
% LED strip position
ledcorners=pos(1:2,:);
for i=3:size(pos,1)-2
  frac=((strutlength/2)/sin(cornerangles/2)) /leglen;
  ledcorners(end+1,:)=pos(i-1,:)*frac+pos(i,:)*(1-frac);
  ledcorners(end+1,:)=pos(i+1,:)*frac+pos(i,:)*(1-frac);
%  fprintf('Length of strut = %.2f\n',norm(ledcorners(end,:)-ledcorners(end-1,:)));
end
ledcorners(end+1,:)=pos(end-1,:);
ledcorners(end+1,:)=pos(end,:);
dled=ledcorners(2:end,:)-ledcorners(1:end-1,:);
ledlength=sum(sqrt(dled(:,1).^2+dled(:,2).^2));
fprintf('Total LED length = %.1fm\n', ledlength);

% Camera positions
camera=[];
cdir=[];
outercameras=1:floor(ncameras/2);
cvertices=unique([2+outercameras,size(pos,1)-outercameras-1]);
if length(cvertices)<ncameras
  cvertices=unique([cvertices,floor(nlegs/2)+2]);
end
for k=2:-1:1
  if length(cvertices)<ncameras
    cvertices=[k,cvertices];
  end
  if length(cvertices)<ncameras
    cvertices=[size(pos,1)-k+1,cvertices];
  end
end
if length(cvertices)~=ncameras
  fprintf('Only able to place %d cameras\n',length(cvertices));
end
for i=cvertices
  camera(end+1,:)=pos(i,:);
  cdir(end+1,:)=-pos(i,:)/norm(pos(i,:));  % Directed towards center
%  fprintf('Placed camera at vertex %d (%.1f,%.1f)\n', i, camera(end,:));
end
  
% LED positions
if isnan(ledspacing)
  ledspacing=ledlength/nleds;
  fprintf('Using LED spacing of %.2f cm (nominal = %.2f) for %d LEDs\n',ledspacing*100,100/32,nleds);
end

seg=1;
lpos=[];
ldir=[];
  
offset=ledspacing/2;   % Initial LED
for seg=1:size(ledcorners,1)-1
  v=ledcorners(seg+1,:)-ledcorners(seg,:);
  seglen=norm(v);
  segpos=offset:ledspacing:seglen;
  v=v/seglen;
  ind=size(lpos,1);
  lpos(ind+1:ind+length(segpos),:)=[ledcorners(seg,1)+segpos*v(1);ledcorners(seg,2)+segpos*v(2)]';
  ldir(ind+1:ind+length(segpos),1)=v(2);
  ldir(ind+1:ind+length(segpos),2)=-v(1);
  offset=segpos(end)+ledspacing-seglen;
end

if doplot
  setfig('layoutpolygon');
  clf;
  hold on;
  title(sprintf('layoutpolygon(%d)',nlegs));
%  plot(lpos(:,1),lpos(:,2),'xm');
  h=plot(pos(:,1),pos(:,2),'r');
  labels={'Main struts'};
  for i=1:size(ledcorners,1)-1
    h(2)=plot(ledcorners([i,i+1],1),ledcorners([i,i+1],2),'g');
  end
  labels{2}='LED struts';
  % Cameras
  for i=1:size(camera,1)
    h(3)=plot(camera(i,1),camera(i,2),'ko');
  end
  labels{3}='Cameras';

  if 0
    % Lines of sight
    h(4)=plot(pos([5,end],1),pos([5,end],2),'m');
    labels{4}='Sightlines';
    plot(pos([5,2],1),pos([5,2],2),'m');
    plot(pos([7,1],1),pos([7,1],2),'m');
    plot(pos([7,end-1],1),pos([7,end-1],2),'m');
  end
  legend(h,labels);
  axis equal
  pause(0);
end

% Setup return variables
layout=struct('cpos',camera,'cdir',cdir,'lpos',lpos,'ldir',ldir,'active',ledcorners,'pos',pos);
