ic=imread('/tmp/pingpong.tif');
ij=imread('/tmp/canvas.tif');
setfig('C++');clf;
imshow(ic(:,:,1:3));
title('C++');
setfig('Java');clf;
imshow(ij);
title('Java');
setfig('map');clf;
lbls={'Red','Green','Blue','Alpha'};
for c=1:3
  ijf=ij(:,:,c);ijf=ijf(:)+1;
  icf=ic(:,:,c); icf=icf(:)+1;
  map=zeros(256,256);
  for i=1:length(ijf)
    map(ijf(i),icf(i))=map(ijf(i),icf(i))+1;
  end
  map(end+1,:)=nan;
  map(:,end+1)=nan;
  map(1,1)=0;
  subplot(2,2,c);
  pcolor(log10(map));
  colorbar;
  title(lbls{c});
  shading flat;
end
subplot(2,2,4);
for i=1:256
  mnj(i)=mean(ijf(icf==i));
end
plot(0:255,mnj-1);
hold on;
plot(0:255,((0:255)/256).^2.2*256,':');
title('Gamma=2.2');
xlabel('C++');
ylabel('Java');


% Compute relative on a pixel-by-pixel basis
rel=log(im2double(ij(:,:,1:3)))./log(im2double(ic(:,:,1:3)));
