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
