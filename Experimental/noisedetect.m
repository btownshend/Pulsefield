% Do noisedetection of a frame
function noisedetect(p,vis,cam)
setfig('noisedet');clf;
nplot=5; pnum=1;

nm=noisemodel(vis,cam);
img=im2double(vis.im{cam});
diff=img-nm.avg;
nstd=abs(diff./nm.std);
%nstd=nstd.^2;
fprintf('Mean sample level: %.1f/255, mean std: %.1f/255\n',255*mean(nm.avg(:)),255*mean(nm.std(:)));
fprintf('Fraction >1 std from avg: %.3f\n',mean(nstd(:)>1));
fprintf('Fraction >2 std from avg: %.3f\n',mean(nstd(:)>2));

subplot(nplot,1,pnum); pnum=pnum+1;
imshow(nstd/max(nstd(:)));
title(sprintf('Unfiltered deviation / %.2f',max(nstd(:))));

subplot(nplot,1,pnum); pnum=pnum+1;
hist(nstd(:),1000);
title('Distribution of nstd');

%f=imfilter(nstd,fspecial('average',[5 7]));
f=imfilter(nstd,fspecial('average',[12 5]));
fs=f;
for i=1:3
  fs(:,:,i)=f(:,:,i)/max(max(f(:,:,i)));
end

subplot(nplot,1,pnum); pnum=pnum+1;
imshow(f/max(f(:)));
title(sprintf('Filtered deviation / %.2f',max(f(:))));

thresh=1.5;
%thresh=2.25;
det=f>thresh;
rgb=img;
rgb=max(rgb,det);
subplot(nplot,1,pnum); pnum=pnum+1;
imshow(rgb);
title(sprintf('Deviation>%.2f',thresh));

bwdet=det(:,:,1)|det(:,:,2)|det(:,:,3);
subplot(nplot,1,pnum); pnum=pnum+1;
imshow(bwdet);
title('Binary detection');

%subplot(nplot,1,pnum); pnum=pnum+1;
%hist(f(:),1000);
%title('Distribution of f');

% Overall plot with LED decisions overlaid
setfig('noisedet2');clf;
subplot(211);
%imshow(nstd/max(nstd(:)));
imshow(f/max(f(:)));
hold on;
pc=p.camera(cam).pixcalib;
roi=p.camera(cam).roi([1,3]);
for l=1:size(vis.corr,2)
  pl=pc(l).pos-roi;
  if vis.corr(cam,l)>0.5
    plot(pl(1),pl(2),'g');
  elseif ~isnan(vis.corr(cam,l))
    plot(pl(1),pl(2),'r');
  end
  if ~any(isnan(pl))
    pf(l,:)=f(pl(2),pl(1),:);
  else
    pf(l,1:3)=nan;
  end
end
subplot(212);
plot(vis.corr(cam,:),'m');
hold on;
plot(1-pf/3);


