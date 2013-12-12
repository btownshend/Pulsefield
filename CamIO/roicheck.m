roi=p.camera(1).roi;
am=aremulti(1,'av10115-half',{roi});
amfull=aremulti(1,'av10115-half');
amsel=amfull{1}(roi(3):roi(4)-1,roi(1):roi(2)-1,:);
figure;
subplot(311);
yr=(1:64)+25;
xr=(1:64)+510;
imshow(am{1}(yr,xr,:));
title('with roi');
subplot(312);
imshow(amsel(yr,xr,:));
title('from full image');
subplot(313);
imshow(vis.im{1}(yr,xr,:));
title('vis.im');
xx
y0=608;y1=704
x1=1792;
for x0=222:226
  am=aremulti(1,'av10115-half',{[x0,x1,y0,y1]+1});
  fprintf('x0=%d, x1=%d, diff=%d => %d\n', x0, x1, x1-x0, size(am{1},2));
  sz(x0+1,:)=size(am{1});
end
