% Check alignment of pixcalib by looking at brightness distribution over window
% Useful for debugging odd behaviours of particular LEDs
% Usage: checkalignment(p,leds,cameras,figname)
%  p - pstruct
%  leds - list of LED numbers to display (default: all)
%  camera - list of camera numbers (default: all)
%  figname - arg to setfig to keep a separate figure
function allsum=checkalignment(p,leds,cameras,figname)
rescale=true;  % Rescale images so maximum pixel=1.0
if nargin<2
  nl=numled();
  leds=1:numled();
else
  nl=length(leds);
end

if nargin<3
  nc=length(p.camera);
  cameras=1:nc;
else
  nc=length(cameras);
end

if nargin<4
  setfig('checkalignment');
else
  setfig(figname);
end
clf;
allsum={};

for ci=1:nc
  c=cameras(ci);
  pixcalib=p.camera(c).pixcalib;
  vc=p.camera(c).viscache;

  fvalid=find([pixcalib.valid]);
  fprintf('Camera %d, LEDs = %s\n',c,shortlist(fvalid));
  first=1;
  cx=nan(1,nl);
  cy=nan(1,nl);
  for i=1:nl
    l=leds(i);
    if ~pixcalib(l).valid
      continue;
    end

    refim=p.camera(c).viscache.refim(vc.tlpos(l,2):vc.brpos(l,2),vc.tlpos(l,1):vc.brpos(l,1),:);
    gray=sum(refim,3);
    xsum=sum(gray);
    %[~,cx(i)]=max(xsum);
    cx(i)=dot(xsum-min(xsum),1:length(xsum))/sum(xsum-min(xsum));
    cx(i)=cx(i)-length(xsum)/2;
    ysum=sum(gray');
    [~,cy(i)]=max(ysum);
    %cy(i)=dot(ysum-min(ysum),1:length(ysum))/sum(ysum-min(ysum));
    cy(i)=cy(i)-length(ysum)/2;
    %dot(ysum,1:length(ysum))/sum(ysum)-length(ysum)/2;
    allsum{ci,i}={xsum,ysum};
  end
  subplot(nc,2,ci*2-1);
  plot(leds,cx);
  hold on;
  plot(leds,0*leds,':r');
  %axis([1,max(leds),-5,5]);
  xlabel('LED Number');
  ylabel('Offset (pixels)');
  title(sprintf('X Offset for Camera %d',c));
  subplot(nc,2,ci*2);
  plot(leds,cy);
  hold on
  plot(leds,0*leds,':r');
  %axis([1,max(leds),-3,3]);
  xlabel('LED Number');
  ylabel('Offset (pixels)');
  title(sprintf('Y Offset for Camera %d',c));
end
