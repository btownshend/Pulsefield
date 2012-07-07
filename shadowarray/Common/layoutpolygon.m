% layoutoctagon - compute position of LED's and cameras in octagon setup (with 1m entry opening)
% nlegs - number of sides of polygon
% doplot - 1 to plot
function layout=layoutpolygon(nlegs,ncameras,nleds,doplot)
if nargin<4
  doplot=0;
end
% Work in meters
inchpermeter=39.3700787;
feetpermeter=inchpermeter/12;
conlen=(1+1/16)/inchpermeter;   % Length that connectors add to legs
leglen=96.5/inchpermeter+2*conlen;
strutlength=4/feetpermeter;
entrylength=conlen+4/feetpermeter;  % First LED is ledspacing(1) in from the end of 4' member
opening=2/feetpermeter;   % 24" opening

% Spacing between LED and prior one (including post-space after last one)
ledspacing=1/32*ones(1,numled());   % May be overridden later by setting of nleds
% Extra gaps at end of each strip
firstleds=cumsum(numled(1:3));
ledspacing(firstleds)=[6,5.6,5.9]/100;  
ledspacing(1)=0.5/inchpermeter;  % First LED inset from end of 4' member by 0.5in
ledspacing(end+1)=ledspacing(1);  % Assume same space after last LED

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
cpos=[];
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
  cpos(end+1,:)=pos(i,:);
  cdir(end+1,:)=-pos(i,:)/norm(pos(i,:));  % Directed towards center
%  fprintf('Placed camera at vertex %d (%.1f,%.1f)\n', i, cpos(end,:));
end


% LED positions
if nargin>=3 && ~isempty(nleds)
  rescale=ledlength/sum(ledspacing);
  fprintf('Rescaling led spacing by %.3f for %d LEDs\n',rescale,nleds);
  ledspacing=ledspacing*rescale;
end

seg=1;
lpos=nan(nleds,2);
ldir=nan(nleds,2);
  
offset=ledspacing(1);   % Initial LED
lnum=1;
for seg=1:size(ledcorners,1)-1
  v=ledcorners(seg+1,:)-ledcorners(seg,:);
  seglen=norm(v);
  v=v/seglen;
  while offset<=seglen && lnum<=nleds
    lpos(lnum,:)=ledcorners(seg,:)+v*offset;
    ldir(lnum,:)=[v(2),-v(1)];
    lnum=lnum+1;
    offset=offset+ledspacing(lnum);
  end
  offset=offset-seglen;
end
if size(lpos,1)~=numled()
  error('Placed %d LEDs instead of expected %d', size(lpos,1),numled());
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
  for i=1:size(cpos,1)
    h(3)=plot(cpos(i,1),cpos(i,2),'ko');
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

active=ledcorners;
% At entry use intersection of camera 2,3 sightlines to last leds
active(end+1,:)=lineintersect(cpos(3,:),lpos(1,:),cpos(2,:),lpos(end,:));
% Setup return variables
layout=struct('cpos',cpos,'cdir',cdir,'lpos',lpos,'ldir',ldir,'active',active,'pos',pos);
