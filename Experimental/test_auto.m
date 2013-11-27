%target=struct('dX',0.09,'dY',0.09,'nX',3,'nY',4);
target=struct('dX',0.03,'dY',0.03,'nX',16,'nY',16);
if ~exist('cdata')
  for cam=1:2
    d=load(sprintf('c%d-Results_left',cam));
    I=arecont(cam);
    setfig('interactive');
    imshow(I.im);
    title('Click on top left, bottom right corners');
    [cx,cy]=ginput(2);
    I.bounds=round([cx,cy]);
    plot(I.bounds(:,1),I.bounds(:,2),'x');
    r=extrinsic_auto(I,target,d,0,0);
    position=cam2grid(r,[0;0;0]);
    fprintf('Camera %d position is [%.2f,%.2f,%.2f]\n', cam, position);
    cdata(cam)=struct('camera',cam,'position',position,'r',r,'target',target,'image',I);
  end
end

for c=1:length(cdata)
  for c2=c+1:length(cdata)
    fprintf('Distance between camera %d and camera %d = %.2f m\n', cdata(c).camera, cdata(c2).camera, norm(cdata(c).position-cdata(c2).position));
  end
end

setfig('Grid space');clf;
hold on;
plotworld(cdata);

setfig('World space');clf;
hold on;
world=makeworld(cdata);
plotworld(cdata,world);
axis([-5,5,0,10,0,2]);

