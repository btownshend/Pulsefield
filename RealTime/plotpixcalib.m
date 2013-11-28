% Plot results of pixcalibrate
function plotpixcalib(cam)
setfig(['pixcalibplot.diffs',cam.id]);
clf;
cp=reshape([cam.pixcalib.pos],2,[]);
dcp=0;
for i=2:size(cp,2)
  dcp(i)=norm(cp(:,i)-cp(:,i-1));
end
dcp(~[cam.pixcalib.valid])=nan;
plot((1:size(cp,2)-1)+0.5,dcp(2:end),'.');
xlabel('LED');
ylabel('Pixels');
title(sprintf('Camera %d: LED separation/diameter',cam.id));
hold on;
plot([cam.pixcalib.diameter],'-r');
legend('Separation','Diameter');

% Plot centroids
setfig(['pixcalibplot.centroids',cam.id]);
clf;
hold on;
axis ij;
axis equal
if isfield('cam.pixcalib','stats')
  for i=1:length(cam.pixcalib)
    stats=cam.pixcalib(i).stats;
    for j=1:length(stats)
      c=stats(j).Centroid;
      d=stats(j).EquivDiameter;
      rectangle('Position',[c-d/2,d,d],'Curvature',[1,1],'FaceColor','r');
      %    scatter(c(1),c(2),stats(j).Area);
      text(c(1),c(2),sprintf('%d',i),'HorizontalAlignment','center');
    end
  end
end
plot(cam.roi(1:2),cam.roi([3,3]),':');
plot(cam.roi(1:2),cam.roi([4,4]),':');
plot(cam.roi([1,1]),cam.roi(3:4),':');
plot(cam.roi([2,2]),cam.roi(3:4),':');
title(sprintf('Camera %d: LED all detected pixel positions',cam.id));
xlabel('Pixels');
ylabel('Pixels');

% Plot final pos
setfig(['pixcalibplot.pos',cam.id]);
clf;
hold on;
axis ij;
axis equal
for i=1:length(cam.pixcalib)
  if cam.pixcalib(i).valid
    c=cam.pixcalib(i).pos;
    d=cam.pixcalib(i).diameter;
    if ~isfinite(d)
      d=1;
    end
    rectangle('Position',[c-d/2,d,d],'Curvature',[1,1],'FaceColor','r');
    %    scatter(c(1),c(2),stats(j).Area);
    text(c(1),c(2),sprintf('%d',i),'HorizontalAlignment','center');
  end
end
plot(cam.roi(1:2),cam.roi([3,3]),':');
plot(cam.roi(1:2),cam.roi([4,4]),':');
plot(cam.roi([1,1]),cam.roi(3:4),':');
plot(cam.roi([2,2]),cam.roi(3:4),':');
title(sprintf('Camera %d: LED pixel final positions',cam.id));
xlabel('Pixels');
ylabel('Pixels');
