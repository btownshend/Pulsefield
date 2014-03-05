x{1}=1410;
y{1}=1150;
x{2}=2158;
y{2}=1050;
width=150;
k=1;
figure(1);clf;
while true
  for i=1:1
    z=arecont(i,0);
    zr=mean(im2double(z.im(y{i}+(-width:width),x{i}+(-width:width),:)),3);
    subplot(211);
    imshow(zr);
    subplot(212);
    cdfplot(zr(:));
%    axis([0,0.5,0.4,0.6]);
  end
  k=k+1;
  pause(0.1);
end