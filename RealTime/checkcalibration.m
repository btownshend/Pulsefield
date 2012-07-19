% Check if any cameras/LEDs have moved
function checkcalibration(p,vis)
nl=15;
wind=5;
leds=round(1:(numled()-1)/(nl-1):numled());
if nargin<2
  s1=arduino_ip();
  setled(s1,-1,[0,0,0],1);
  setled(s1,leds-1,20*[1 1 1],1);
  show(s1);
  sync(s1);
  pause(2);
  vis=getvisible(p,0);
  setled(s1,-1,[0,0,0],1);
  show(s1);
end

setfig('checkcalibration');
clf;
nc=length(p.camera);
for c=1:nc
  pixcalib=p.camera(c).pixcalib;
  fvalid=find([pixcalib.valid]);
  fprintf('Camera %d, LEDs = %s\n',shortlist(fvalid));
  first=1;
  for i=1:nl
    l=leds(i);
    if pixcalib(l).valid
      subplot(nc,nl,(c-1)*nl+i);
      plist=pixcalib(l).pixelList;
      roi=p.camera(c).roi;
      plist(:,1)=plist(:,1)-roi(1)+1;
      plist(:,2)=plist(:,2)-roi(3)+1;
      for j=1:2
        rng{j}=max(1,min(plist(:,j))-wind):min(size(vis.im{c},3-j),max(plist(:,j))+wind);
      end
      imled=im2double(vis.im{c}(rng{2},rng{1},:));
      imshow(imled);
      %    imshow(imled/max(imled(:)));
      hold on;
      for i=1:size(plist,1)
        plot(-(rng{1}(1)-1)-0.5+plist(i,1)+[0 1 1 ],-(rng{2}(1)-1)-0.5+plist(i,2)+[1 1 0 ],'r');
      end
      if first
        ylabel(sprintf('Camera %d',c));
        first=0;
      end
      title(sprintf('LED %d\n', l));
    end
  end
end