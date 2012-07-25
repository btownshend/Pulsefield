% calcspos - calculate position of LED images on cameras' sensors
function [spos,sa]=calcspos(p,quiet,nobad)
layout=p.layout;
cpos=layout.cpos;
lpos=layout.lpos;
cdir=layout.cdir;
if nargin<2
  quiet=0;
end
if nargin<3
  % nobad indicates that out of range values should NOT be nan-ed
  nobad=0;
end
maxsep=0;
minangle=pi;
spos=[];
pixsep=[];
v=zeros(size(cpos,1),size(lpos,1),2);
sa=zeros(size(cpos,1),size(lpos,1));
bad=false(size(cpos,1),size(lpos,1));

sa=[];
for c=1:size(cpos,1)
  cangle=atan2(cdir(c,2),cdir(c,1));
  v(c,:,1)=lpos(:,1)-cpos(c,1);
  v(c,:,2)=lpos(:,2)-cpos(c,2);
  % Calculate angle to right of center
  sa(c,:)=cangle-atan2(v(c,:,2),v(c,:,1));
  % But if we're seeing a LED from behind, don't count it
  bad(c,:)=(v(c,:,1)'.*layout.ldir(:,1)+v(c,:,2)'.*layout.ldir(:,2))>0;
  sel=sa(c,:)>pi;
  sa(c,sel)=sa(c,sel)-2*pi;
  sel=sa(c,:)<-pi;
  sa(c,sel)=sa(c,sel)+2*pi;
  
  % Note that sa is angle to left of center, whereas angle2spos expects angle to right
  spos(c,:)=angle2spos(sa(c,:),p.camera(c));

  if ~nobad
    sel=spos(c,:)<0 | spos(c,:)>=p.camera(c).hpixels;
    spos(c,sel)=nan;
    spos(c,bad(c,:))=nan;
    sa(c,~isfinite(spos(c,:)))=nan;
  end
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
  dled=diff(lpos);
  ledspacing=median(hypot(dled(:,1),dled(:,2)));
  minangle=(ledspacing/sqrt(2)) / maxsep;
  fprintf('Min distance between LEDs on sensor = %.1f pixels with min angle %.1f deg and max distance %.1fm\n',minpixsep,minangle*180/pi,maxsep);
  nvis=sum(isfinite(spos(:)));
  fprintf('Total of %d LEDs visible (%.0f per camera = %.1f%%)\n', nvis, nvis/size(cpos,1),nvis/size(cpos,1)/size(lpos,1)*100);
  minpixsep=2;
  nvis2=sum(pixsep(:)>minpixsep);
  fprintf('Total of %d LEDs separable by at least %d pixels (%.0f per camera = %.1f%%)\n', nvis2, minpixsep, nvis2/size(cpos,1),nvis2/size(cpos,1)/size(lpos,1)*100);
end

function n=normrows(v)
n=sqrt(v(:,:,1).^2+v(:,:,2).^2);