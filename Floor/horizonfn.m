function p=horizonfn(p)
% Estimate horizon function based on horizon points
for c=1:length(p.camera)
  cam=p.camera(c);

  % Form horizon line, find points corresponding to (fake) LEDs
  x=cam.horizon(:,1);
  y=cam.horizon(:,2);
  poly=polyfit(x,y,2);
  xp=(1:length(p.led))*(cam.hpixels/length(p.led));
  yp=polyval(poly,xp);


  % Setup ROI
  border=5;
  roi=[1,cam.hpixels+1,max(1,floor(min(yp)-border)),min(cam.vpixels+1,ceil(max(yp)+border))];
  % Make divisible by 32 for camera
  roi([1,3])=floor((roi([1,3])-1)/32)*32+1;
  roi([2,4])=ceil((roi([2,4])-1)/32)*32+1;
  p.camera(c).roi=roi;

  % Build pixcalib structure
  pc=[];
  for i=1:length(xp)
    pc=[pc,struct('pos',round([xp(i),yp(i)]),'pixelList',round([xp(i),yp(i)]),'diameter',1,'valid',1,'inuse',1)];
  end
  % Setup indices from pixelList
  for i=1:length(pc)
    if pc(i).valid
      pixelList=pc(i).pixelList;
      pixelList(:,1)=pixelList(:,1)-roi(1)+1;
      pixelList(:,2)=pixelList(:,2)-roi(3)+1;
      % Setup indices from pixellists
      pc(i).indices=...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1)],pixelList(:,2),pixelList(:,1));
      pc(i).rgbindices=[...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1),3],pixelList(:,2),pixelList(:,1),1*ones(size(pixelList,1),1));...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1),3],pixelList(:,2),pixelList(:,1),2*ones(size(pixelList,1),1));...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1),3],pixelList(:,2),pixelList(:,1),3*ones(size(pixelList,1),1))];
    end
  end
  p.camera(c).pixcalib=pc;
end

% Adjust camera direction based on other cameras in field of view
for c=1:length(p.camera)
  cam=p.camera(c);
  if isempty(cam.othercams)
    fprintf('No other cameras in camera %d field of view -- cannot adjust cdir\n', i);
    continue;
  end
  emat=nan(size(cam.othercams,1),length(p.camera));
  dir=emat;
  for j=1:size(cam.othercams,1)
    for c2=1:length(p.camera)
      if c2~=c
        delta=p.layout.cpos(c2,:)-p.layout.cpos(c,:);
        angle=cart2pol(delta(1),delta(2));
        emat(j,c2)=cart2pol(p.layout.cdir(c,1),p.layout.cdir(c,2))-cam.anglemap(round(cam.othercams(j,1)))-angle;
        dir(j,c2)=cart2pol(p.layout.cdir(c,1),p.layout.cdir(c,2))-angle;
      end
    end
  end
  emat(emat>pi)=emat(emat>pi)-2*pi;
  emat(emat<-pi)=emat(emat<-pi)+2*pi;
  dir(dir>pi)=dir(dir>pi)-2*pi;
  dir(dir<-pi)=dir(dir<-pi)+2*pi;
  pe=perms(1:length(p.camera)-1);
  pe(pe>=c)=pe(pe>=c)+1;
  pe=unique(pe(:,1:size(emat,1)),'rows');
  err=[];
  angle=[];
  val=[];
  pdir=[];
  for k=1:size(pe,1)
    for m=1:size(emat,1)
      val(m,k)=emat(m,pe(k,m));
      pdir(m,k)=dir(m,pe(k,m));
    end
    angle(k)=mean(val(:,k));
    err(k)=sum(abs(val(:,k)-mean(val(:,k))));
  end
  [besterr,bestind]=nanmin(err);
  rot=angle(bestind);
  fprintf('Camera %d: Best ordering: %s with rot %.1f, error %s\n', c,sprintf('%d ',pe(bestind,:)),rot*180/pi,sprintf('%.1f ',180/pi*(val(:,bestind)-rot)));
  
  setfig('uerr');
  subplot(length(p.camera),1,c);
  plot(180/pi*(pdir(:,bestind)-rot),180/pi*(val(:,bestind)-rot),'.');
  xlabel('View angle (deg)');
  ylabel('Anglemap error (deg)');
  grid on;
  axis([-90,90,-5,5]);

  [newdir(1),newdir(2)]=pol2cart(cart2pol(p.layout.cdir(c,1),p.layout.cdir(c,2))-rot,1);
  p.layout.cdir(c,:)=newdir;
end
