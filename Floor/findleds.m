%im=acquire(p,20);
radius=10*12/39.37;  % Meters
th=[1.5,2.5,-3.5,-2.5,-1.5,3.5];
cpos=[];
for i=1:length(th)
  [cpos(i,1),cpos(i,2)]=pol2cart((pi/2-th(i)*pi/6),radius);
end
d={};r={};
pulse=[];
for i=1:length(im{1})
  fprintf('Processing camera %d\n',i);
  z=[];
  for k=1:length(im)
    z(:,:,:,k)=im{k}{i};
  end
  r{i}=im2double(max(z,[],4)-min(z,[],4));
end

for i=1:length(r)
  mx=[];
  for j=1:3
    pl=r{i}(:,:,j);
    mx(j)=max(pl(:));
    r{i}(:,:,j)=r{i}(:,:,j)/mx(j);
  end
  fprintf('Camera %d: Max=%.2f %.2f %.2f\n', i, mx);
end
setfig('leds');clf;
rp={};
for i=1:length(r)
  subplot(2,3,i);
  imshow(r{i});
  hold on;
  bw=bwlabel(im2bw(r{i},0.1));
  rp{i}=regionprops(bw,'all');
  rp{i}=rp{i}([rp{i}.Area]>1);
  for k=1:length(rp{i})
    plot(rp{i}(k).Centroid(1),rp{i}(k).Centroid(2),'or');
  end
end
setfig('ledpos');clf;
width=nan(length(rp),1);
for i=1:length(rp)
  subplot(2,3,i);
  for k=1:length(rp{i})
    c=rp{i}(k).Centroid;
    plot(c(1),c(2),'o');
    levs=[];
    for fr=1:length(im)
      levs(fr)=im{fr}{i}(round(c(2)),round(c(1)));
    end
    rp{i}(k).levels=levs;
    rp{i}(k).onframes=levs>(max(levs)+min(levs))/2;
    fprintf('Camera %d, region %d, levs=%s, on=%s\n', i, k, sprintf('%.2f ',levs),sprintf('%d ',rp{i}(k).onframes));
    hold on;
  end
  if length(rp{i})==3
    mn=[1e10,1e10];
    mx=[0,0];
    for j=1:length(rp{i})
      mn(1)=min(mn(1),rp{i}(j).Centroid(1));
      mx(1)=max(mx(1),rp{i}(j).Centroid(1));
      mn(2)=min(mn(2),rp{i}(j).Centroid(2));
      mx(2)=max(mx(2),rp{i}(j).Centroid(2));
    end
    fprintf('Camera %d: width=%.1f\n', i, norm(mx-mn));
    width(i)=norm(mx-mn);
  end
end
setfig('pulses');clf
col='rgbcymk';
for i=1:length(rp)
  subplot(2,3,i);
  for j=1:length(rp{i})
    plot(rp{i}(j).levels,col(j));
    hold on;
  end
end

setfig('layout');clf;
sc=135;
for i=1:size(cpos,1)
  plot(cpos(i,1),cpos(i,2),'o');
  hold on;
  text(cpos(i,1)+.1,cpos(i,2),sprintf('%d',i));
  trace=[];
  if isfinite(width(i))
    fprintf('Tracing %d\n',i);
    [trace(:,1),trace(:,2)]=pol2cart(0:.1:(2*pi+0.1),sc/width(i));
    plot(cpos(i,1)+trace(:,1),cpos(i,2)+trace(:,2));
  end
end
axis equal;
axis([-radius,radius,-radius,radius]);

