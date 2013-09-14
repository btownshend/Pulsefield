function plothorizon(p,im)
if nargin<2
  im=aremulti([p.camera.id],p.camera(1).type);
end
setfig('PlotHorizon');clf;
for c=1:length(p.camera)
  roi=p.camera(c).roi;
  subplot(length(p.camera),1,c);
  imshow(adapthisteq(rgb2gray(im{c}(roi(3):roi(4)-1,roi(1):roi(2)-1,:)),'ClipLimit',.2));
  hold on;
  pc=p.camera(c).pixcalib;
  for i=1:length(pc)
    plot(pc(i).pos(1)-roi(1),pc(i).pos(2)-roi(3),'go');
  end
  plot(p.camera(c).horizon(:,1)-roi(1),p.camera(c).horizon(:,2)-roi(3),'ro');
end
