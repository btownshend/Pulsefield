% Calculate background noise model using a set of background frames
function nm=noisemodel(bg,cam)
for i=1:length(bg)
  a=bg{i};
  im=im2double(a{cam});
  if i==1
    s=im;
    s2=im.^2;
  else
    s=s+im;
    s2=s2+im.^2;
  end
end
N=length(bg);
nm.avg=s/N;
nm.std=sqrt(s2-s.^2/N);
% Make sure the std isn't too small -- use 1/3 of mean as clip
fprintf('Clipping at %.3f\n', mean(nm.std(:))/3);
nm.std=max(nm.std,mean(nm.std(:))/3);
