% Test finding translation, rotation of a set of point correspondences that maps wpts->lpts (A*wpts=lpts)
lpos=[1,-4];  % LIDAR position in world coords
laim=[0,0];  % World position LIDAR aimed at
dir=laim-lpos;
lrot=atan2(dir(2),dir(1))-pi/2;
mat1=[cos(lrot) -sin(lrot) lpos(1)
      sin(lrot) cos(lrot) lpos(2)
      0 0 1];
lpts=[0, 0
      0,norm(lpos)
      -10,0
      -10,10
      10,10
      10,0]
lptsa=lpts; lptsa(:,3)=1;
wptsa=mat1*lptsa';
wpts=wptsa(1:2,:)';
[mat2,f]=findTranslateRotate(lpts,wpts);
mat2
rotate2=atan2(mat2(2,1),mat2(1,1));
translate2=mat2(1:2,3);
fprintf('Translate: [%f %f]\n', translate2);
fprintf('Rotate: %f\n',  rotate2);

setfig('LIDAR');clf;
plot(lpts([1:end,1],1),lpts([1:end,1],2),'o-');
hold on;
plot(wpts([1:end,1],1),wpts([1:end,1],2),'x-');
axis equal
legend('LIDAR','World');
return;

noise=0.01;
while true
  npoints=randi(3,1,1)+1;
  fprintf('------\nUsing %d points\n',npoints);
  translate=randn(1,2);
  rotate=(rand(1)*2-1)*pi;
  mat=[cos(rotate) sin(rotate) translate(1)
       -sin(rotate) cos(rotate) translate(2)
       0 0 1];

  wpts=randn(npoints,2);
  wptsA=wpts; wptsA(:,3)=1;
  lptsA=(mat*wptsA')';
  lpts=lptsA(:,1:2);
  lpts=lpts+randn(size(lpts))*noise;   % Some noise
                                    %lpts(:,2)=-lpts(:,2);   % Change of handedness
  [mat2,f]=findTranslateRotate(wpts,lpts);

  rotate2=atan2(mat2(1,2),mat2(1,1));
  translate2=mat2(1:2,3);
  fprintf('Translate: [%f %f] -> [%f %f]\n', translate, translate2);
  fprintf('Rotate: %f -> %f\n', rotate, rotate2);
  lpts2A=(mat2*wptsA')';
  lpts2=lpts2A(:,1:2);
  diff=lpts-lpts2;
  for i=1:size(wpts,1)
    fprintf('wpts=[%5.2f %5.2f], lpts=[%5.2f %5.2f], A*wpts=[%5.2f %5.2f], err=%5.2f\n', wpts(i,:), lpts(i,:), lpts2(i,:), norm(diff(i,:)));
  end

  rms=sqrt(mean(diff(:).^2));
  fprintf('RMS=%5.3f\n',rms);
  if f
    pause(1);
  end
  if rms>2*noise
    break;
  end
end

