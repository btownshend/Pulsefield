% Adjust positions of LEDs based on pixel calibration
function calib=adjpos(p,layout)
% Form spos
spos=zeros(length(p.camera),length(p.led));
for i=1:length(p.camera)
  for j=1:length(p.led)
    spos(i,j)=p.camera(i).pixcalib(j).pos(1);   % Use x-coord only
  end
end
% Plot values compared to theoretical
for i=1:length(p.camera)
  setfig(sprintf('adjspos%d',i));
  clf;
  plot(spos(i,:));
  hold on;
  sposdesign=calcspos(p,layout);
  plot(sposdesign(i,:),'r');
  xlabel('LED');
  ylabel('x-pixel value');
  legend('Observed','Theoretical');
  title(sprintf('Camera %d',i));
end
% Calibrate
calib=calibrate(p,layout,spos,1);
