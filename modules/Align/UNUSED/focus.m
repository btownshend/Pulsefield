% Display camera image to allow focussing
p=struct('camera',struct('id',1,'physid',1));
setupcameras(p,'exptime',80,'analoggain',10);
while true
  tmp=arecont(1,1);
  imshow(tmp.im);
  figure(gcf);
  pause(0.1);
end
