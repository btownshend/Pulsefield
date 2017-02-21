dir='/tmp/';
iv=im2double(imread([dir,'/ofcapt-vel.tif']));
id=im2double(imread([dir,'/ofcapt-den.tif']));
id2=im2double(imread([dir,'/ofcapt-den2.tif']));
ip=im2double(imread([dir,'/ofcapt-press.tif']));
ip=ip(:,:,1);
it=im2double(imread([dir,'/ofcapt-temp.tif']));
it=it(:,:,1);
idiv=im2double(imread([dir,'/ofcapt-div.tif']));
idiv=idiv(:,:,1);
rng=[min(min(iv(:,:,1))),max(max(iv(:,:,1))),min(min(iv(:,:,2))),max(max(iv(:,:,2)))];
fprintf('Velocity range: [%.2f:%.2f, %.2f:%.2f]\n', rng);
maxvel=max([rng,-rng]);

setfig('Images');clf;
subplot(221);
quiver(iv(:,:,1),iv(:,:,2));
axis ij;
axis equal;
axis([0,size(iv,2),0,size(iv,1)]);

%imshow(iv(:,:,1),'DisplayRange',[-maxvel,maxvel]);
title('Velocity');
subplot(222);
iv(:,:,3)=0;
imshow((iv(:,:,1:3)-min(rng))/(max(rng)-min(rng)));
title('Velocity');
subplot(223);
imshow(id(:,:,1:3));
title('Density');
subplot(224);
imshow(ip,'DisplayRange',[min(ip(:)),max(ip(:))]);
title('Pressure');

setfig('Divergence');clf;
subplot(121);
div=divergence(iv);
plotdiv(div,'Comp.');

subplot(122);
plotdiv(idiv*1.25,'Read');

[vfinal,pfinal]=project(iv);
divfinal=divergence(vfinal);
setfig('Final');clf;
subplot(2,2,[1,3]);
quiver(vfinal(:,:,1),vfinal(:,:,2));
axis ij;
axis equal;
axis([0,size(vfinal,2),0,size(vfinal,1)]);
title('Velocity');
subplot(222)
plotdiv(divfinal,'Final');
title('Divergence');
subplot(224)
imshow(pfinal,[min(pfinal(:)),max(pfinal(:))]);
title('Pressure');
suptitle('Final');

setfig('denscheck');
subplot(121);
imshow(id(:,:,1:3));
title('Den');
subplot(122);
imshow(id2(:,:,1:3));
title('Den2');

function plotdiv(div,name)
ii=div(:,:,1);
imshow(ii,'DisplayRange',[min(ii(:)),max(ii(:))]);
title(sprintf('%s Divergence [%.2f:%.2f] RMS=%.2f',name, min(ii(:)),max(ii(:)),sqrt(mean(ii(:).^2))));
end


function div=divergence(vel)
cellsize=1;
div=nan(size(vel,1),size(vel,2));
div(2:end-1,2:end-1)=vel(3:end,2:end-1,2)-vel(1:end-2,2:end-1,2)+vel(2:end-1,3:end,1)-vel(2:end-1,1:end-2,1);
div([1,end],:)=0;div(:,[1,end])=0;
div=div*0.5/cellsize;
end

function p=jacobi(div,p)
  alpha=-1;
  beta=4;
  p(2:end-1,2:end-1)=(p(1:end-2,2:end-1)+p(3:end,2:end-1)+p(2:end-1,1:end-2)+p(2:end-1,3:end)+alpha*div(2:end-1,2:end-1))/beta;
  p(1,:)=p(2,:);
  p(end,:)=p(end-1,:);
  p(:,1)=p(:,2);
  p(:,end)=p(:,end-1);
end

function [v,p]=project(v0)
  v0([1,end],:,2)=-v0([2,end-1],:,2);
  v0(:,[1,end],1)=-v0(:,[2,end-1],1);
  div=divergence(v0);
  p=zeros(size(div));
  for i=1:100
    oldp=p;
    p=jacobi(div,p);
    if mod(i,1)==0
      change=sqrt(mean((p(:)-oldp(:)).^2));
      v=v0;
      v(2:end-1,2:end-1,1)=v(2:end-1,2:end-1,1)-0.5*(p(2:end-1,3:end)-p(2:end-1,1:end-2));
      v(2:end-1,2:end-1,2)=v(2:end-1,2:end-1,2)-0.5*(p(3:end,2:end-1)-p(1:end-2,2:end-1));
      v([1,end],:,2)=-v([2,end-1],:,2);
      v(:,[1,end],1)=-v(:,[2,end-1],1);
      newdiv=divergence(v);
      fprintf('%d: rms(dp)=%.2f, rms(div)=%.2f\n', i, change, sqrt(mean(newdiv(:).^2)));
    end
  end
end