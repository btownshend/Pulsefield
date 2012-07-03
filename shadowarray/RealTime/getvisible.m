% getvisible - see what LEDs are visible to the cameras
% Returns v(ncam,nled) = 1 if LED is visible, 0 if not
function [v,lev]=getvisible(sainfo,doplot)
if nargin<2
  doplot=0;
end
% Turn on all LED's
s1=arduino_ip();
cmd=[];
for i=1:length(sainfo.led)
  cmd=[cmd,setled(s1,sainfo.led(i).id-1,[50,50,50],0)];
end
awrite(s1,cmd);
show(s1);
sync(s1);
% even sending second sync does not ensure that the strip has been set
% pause for 300ms (200ms sometimes wasn't long enough)
pause(0.3);
rois={sainfo.camera.roi};
im=aremulti([sainfo.camera.id],rois);
% Turn off LEDs
setled(s1,-1,[0,0,0]);
show(s1);
sync(s1);
if doplot
  setfig('getvisible');
  clf;
end
lev=nan(length(sainfo.camera),length(sainfo.camera(1).pixcalib));
for i=1:length(im)
  if doplot
    subplot(length(im),2,i*2-1)
    imshow(im{i});
    axis normal
    hold on;
    title(sprintf('Camera %d',sainfo.camera(i).id));
    pause(0.01);
  end
  c=sainfo.camera(i).pixcalib;
  roi=sainfo.camera(i).roi;
  imgray=rgb2gray(im2single(im{i}));
  for j=1:length(c)
    if ~isnan(c(j).diameter)
      if doplot
        plot(c(j).pos(1)-roi(1)+1,c(j).pos(2)-roi(3)+1,'or');
      end
      plist=c(j).pixelList;
      plist(:,1)=plist(:,1)-roi(1)+1;
      plist(:,2)=plist(:,2)-roi(3)+1;
      good=plist(:,1)>0&plist(:,2)>0&plist(:,2)<=size(imgray,1)&plist(:,1)<size(imgray,2);
      if sum(~good)>0
        fprintf('Missing %d pixels for LED %d from camera %d\n', sum(~good),j,i);
      end
      plist=plist(good,:);
      indices=sub2ind(size(imgray),plist(:,2),plist(:,1));
      lev(i,j)=sum(imgray(indices))/c(j).difftotal;
    end
  end
  if doplot
    subplot(length(im),2,i*2);
    plot(lev(i,:));
    xlabel('LED');
    ylabel('Signal');
  end
end
v=lev>0.5;

