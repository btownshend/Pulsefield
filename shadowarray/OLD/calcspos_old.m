% calcspos - calculate position of LED images on cameras' sensors
function spos=calcspos_old(cpos,cdir,lpos,p,quiet)
if nargin<5
  quiet=0;
end
maxsep=0;
minangle=pi;
spos=[];
pixsep=[];
for c=1:size(cpos,1)
  cangle=atan2(cdir(c,2),cdir(c,1));
  for l=1:size(lpos,1)
    v=lpos(l,:)-cpos(c,:);
    maxsep=max([maxsep,norm(v)]);
    angle=atan2(v(2),v(1))-cangle;
    if angle>pi
      angle=angle-2*pi;
    elseif angle<-pi
      angle=angle+2*pi;
    end
    angs(c,l)=angle;
    if p.cam.fisheye
      spos(c,l)=p.cam.hpixels*(angle/p.cam.fov+0.5);
    else
      spos(c,l)=p.cam.hpixels*(tan(2*angle/p.cam.fov*pi/4)/2+0.5);
    end
    if spos(c,l)<0 || spos(c,l)>=p.cam.hpixels
      spos(c,l)=nan;
    end
    if l>=2
      minangle=min([minangle,abs(angle-lastangle)]);
    end
    lastangle=angle;
    % But if we're seeing a LED from behind, don't count it
    ldir=[0.5,0.5]*p.sidelength - lpos(l,:);
    if dot(v,ldir)>0
      % Skip this one
      spos(c,l)=nan;
    end
  end
end
if ~quiet
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
