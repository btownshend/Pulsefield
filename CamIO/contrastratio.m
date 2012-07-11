arduino_start;

nled=160;
rgbsensor=0;
level=127;
red=level*[1,0,0];
green=level*[0,1,0];
blue=level*[0,0,1];
white=level/2*[1,1,1];
black=level*[0,0,0];

setled(s1,-1,black);
sync(s1);

colors={red,green,blue,white,black};
colnames={'Red','Green','Blue','White'};
leds=95:120;
%roi={850:960,850:1100};
%roi={700:1200,900:2000};
%roi={900:1050,1500:1900};
%roi={1300:2200,1600:2600};
if ~exist('im')
  setfig('imcapture');
  clf;
  im={};
  info={};
  nskip=min(4,floor(length(leds)/2));
  for c=1:nskip+1
    setled(s1,-1,black);
    if c>1
      for i=1:(c-1):length(leds)
        led=leds(i);
        setled(s1,led,white);
      end
    end
    sync(s1);
    pause(1);
    % Get image
    %[imtmp,info{c}]=dcsget('192.168.0.136:12345/image.jpg');
     imtmp=fwget();
%     p=arecont(id);
    %imtmp=dcsget();
%     imtmp=p.im;
     
    if exist('roi')
      im{c}=im2single(imtmp(roi{1},roi{2},:));
    else
      im{c}=im2single(imtmp);
    end
    if c>1
      imdiff{c-1}=im{c}-im{1};
    end
    subplot(ceil((nskip+1)/3),3,c);
    imshow(im{c});
    nsat=sum(im{c}(:)>0.95);
    if nsat>0
      fprintf('Warning im{%d} saturated at %d pixels\n',c, nsat);
    end
  end
  setled(s1,-1,black);
  sync(s1);
end

sc=1/max(im{2}(:));
fprintf('Scaling by %.2f\n', sc);
for i=1:length(im)
  im{i}=im{i}*sc;
end
for i=1:length(imdiff)
  imdiff{i}=imdiff{i}*sc;
end
% Find the line of LEDs
thresh=0.2;
bwim=imerode(im2bw(imdiff{1},thresh),strel('disk',1));
sz=[size(imdiff{1},1),size(imdiff{1},2)];
[y,x]=ind2sub(sz,find(bwim));
reg=polyfit(x,y,1);
xval=x(1):x(end);
yval=reg(1)*xval+reg(2);
ang=-atan(reg(1))*180/pi;
fprintf('Rotate images CCW by %.1f degrees\n', ang);
setfig('lineplot');
clf
corder=[1 0 0
        0 1 0
        0 0 1];
set(gcf,'DefaultAxesColorOrder',corder);
[theta,r]=cart2pol(xval-sz(2)/2,yval-sz(1)/2);
theta=theta+ang*pi/180;
[xvalr,yvalr]=pol2cart(theta,r);
xvalr=xvalr+sz(2)/2;
yvalr=yvalr+sz(1)/2;
border=25;
for i=1:length(imdiff)
  subplot(length(imdiff),3,i*3-2);
  rotim=imrotate(imdiff{i},-ang,'bilinear','crop');
  rotim=rotim(floor(min(yvalr))-border:ceil(max(yvalr))+border,floor(min(xvalr))-border:ceil(max(xvalr))+border,:);
  rotim=rotim/max(rotim(:));
  imshow(rotim);
%  axis([min(xvalr)-5,max(xvalr)+5,min(yvalr)-5,max(yvalr)+5]);
  hold on;
  plot(xvalr,yvalr,'r:');
  title(sprintf('nskip=%d\n',i-1));
  subplot(length(imdiff),3,i*3-1);
  plot(squeeze(sum(rotim,1)));
  xlabel('Pixel position');
  ylabel('Signal (RGB)');

  subplot(length(imdiff),3,i*3);
  plot(squeeze(sum(sum(rotim,1),3)),'k');
  xlabel('Pixel position');
  ylabel('Signal (Total)');
end
