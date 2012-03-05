% calcspos - calculate position of LED images on cameras' sensors
function [spos,sa]=calcspos(cpos,cdir,lpos,p,quiet,nobad)
if nargin<5
  quiet=0;
end
if nargin<6
  % nobad indicates that out of range values should NOT be nan-ed
  nobad=0;
end
maxsep=0;
minangle=pi;
spos=[];
pixsep=[];
ldir(:,1)=0.5*p.sidelength - lpos(:,1);
ldir(:,2)=0.5*p.sidelength - lpos(:,2);
v=zeros(size(cpos,1),size(lpos,1),2);
sa=zeros(size(cpos,1),size(lpos,1));
bad=false(size(cpos,1),size(lpos,1));

for c=1:size(cpos,1)
  cangle=atan2(cdir(c,2),cdir(c,1));
  v(c,:,1)=lpos(:,1)-cpos(c,1);
  v(c,:,2)=lpos(:,2)-cpos(c,2);
  sa(c,:)=atan2(v(c,:,2),v(c,:,1))-cangle;
  % But if we're seeing a LED from behind, don't count it
  bad(c,:)=(v(c,:,1)'.*ldir(:,1)+v(c,:,2)'.*ldir(:,2))>0;
end
sel=sa>pi;
sa(sel)=sa(sel)-2*pi;
sel=sa<-pi;
sa(sel)=sa(sel)+2*pi;

spos=interp1(p.cam.anglemap,1:p.cam.hpixels,sa);
if 0
if p.cam.fisheye
  spos=p.cam.hpixels*(sa/p.cam.fov+0.5);
else
  spos=p.cam.hpixels*(tan(2*sa/p.cam.fov*pi/4)/2+0.5);
end
end

if ~nobad
  sel=spos<0 | spos>=p.cam.hpixels;
  spos(sel)=nan;
  for c=1:size(cpos,1)
    spos(c,bad(c,:))=nan;
  end
  sa(~isfinite(spos))=nan;
end

if ~quiet
  maxsep=max(max(normrows(v)));
  pixsep=nan(size(spos));
  pixsep(:,2:end)=abs(diff(spos')');
  pixsep(~isfinite(pixsep))=100;
  pixsep(:,2:end-1)=min(pixsep(:,2:end-1),pixsep(:,3:end));
  pixsep(:,1)=min(pixsep(:,1),pixsep(:,end));
  pixsep(:,end)=pixsep(:,1);
  pixsep(~isfinite(spos))=nan;
  minpixsep=min(pixsep(isfinite(pixsep)));
  minangle=(p.ledspacing/sqrt(2)) / maxsep;
  fprintf('Min distance between LEDs on sensor = %.1f pixels with min angle %.1f deg and max distance %.1fm\n',minpixsep,minangle*180/pi,maxsep/p.scale);
  nvis=sum(isfinite(spos(:)));
  fprintf('Total of %d LEDs visible (%.0f per camera = %.1f%%)\n', nvis, nvis/size(cpos,1),nvis/size(cpos,1)/size(lpos,1)*100);
  minpixsep=2;
  nvis2=sum(pixsep(:)>minpixsep);
  fprintf('Total of %d LEDs separable by at least %d pixels (%.0f per camera = %.1f%%)\n', nvis2, minpixsep, nvis2/size(cpos,1),nvis2/size(cpos,1)/size(lpos,1)*100);
end

function n=normrows(v)
n=sqrt(v(:,:,1).^2+v(:,:,2).^2);