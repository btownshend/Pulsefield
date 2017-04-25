% Test finding translation, rotation of a set of point correspondences that maps src->dst (A*src=dst)
dst=[0, 0
      9.515419, 10.770926
 12.687224, 9.9118938];
src=[-12.687224, 16.453745
     0, 45
     25.770926, 4.8898673];
[mat2,f]=findTranslateRotate(src,dst);
mat2
rotate2=atan2(mat2(1,2),mat2(1,1));
translate2=mat2(1:2,3);
fprintf('Translate: [%f %f]\n', translate2);
fprintf('Rotate: %f\n',  rotate2);
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

  src=randn(npoints,2);
  srcA=src; srcA(:,3)=1;
  dstA=(mat*srcA')';
  dst=dstA(:,1:2);
  dst=dst+randn(size(dst))*noise;   % Some noise
                                    %dst(:,2)=-dst(:,2);   % Change of handedness
  [mat2,f]=findTranslateRotate(src,dst);

  rotate2=atan2(mat2(1,2),mat2(1,1));
  translate2=mat2(1:2,3);
  fprintf('Translate: [%f %f] -> [%f %f]\n', translate, translate2);
  fprintf('Rotate: %f -> %f\n', rotate, rotate2);
  dst2A=(mat2*srcA')';
  dst2=dst2A(:,1:2);
  diff=dst-dst2;
  for i=1:size(src,1)
    fprintf('src=[%5.2f %5.2f], dst=[%5.2f %5.2f], A*src=[%5.2f %5.2f], err=%5.2f\n', src(i,:), dst(i,:), dst2(i,:), norm(diff(i,:)));
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

