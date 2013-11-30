% Do noisedetection of a frame
function noisedetect(bg,vis,cam)
nm=noisemodel(bg,cam);
img=im2double(vis.im{cam});
diff=img-nm.avg;
nstd=diff./nm.std;
f=imfilter(nstd,fspecial('average',[9 9]));
f=abs(f);
setfig('noisedet');clf;
fs=f;
for i=1:3
  fs(:,:,i)=f(:,:,i)/max(max(f(:,:,i)));
end
imshow(fs);
keyboard
det=f>0.5;
rgb=img;
rgb=max(rgb,det);
imshow(rgb);
keyboard;
bwdet=det(:,:,1)|det(:,:,2)|det(:,:,3);
imshow(bwdet);
keyboard;
