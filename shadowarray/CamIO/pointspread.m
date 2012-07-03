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
leds=[100,110,120];
%roi={890:960,850:980};
roi={700:1200,900:2000};
if ~exist('im')
  setfig('imcapture');
  im={};
  for i=1:length(leds)
    led=leds(i);
    for c=1:length(colors)
      fprintf('Setting LED %d to (%d,%d,%d)\n', led, colors{c});
      setled(s1,led,colors{c});
      if (i==2)
        fprintf('Setting LEDS %d and %d on together\n', led, led+2);
        setled(s1,led+2,colors{c});
      end
      sync(s1);
      pause(1);
      % Get image
%      imtmp=dcsget();
      imtmp=fwget();
      im{i,c}=im2single(imtmp(roi{1},roi{2},:));
      subplot(length(colors),length(leds),(c-1)*length(leds)+i);
      imshow(im{i,c});
      nsat=sum(im{i,c}(:)>0.95);
      if nsat>0
        fprintf('Warning im{%d,%d} saturated at %d pixels\n', i, c, nsat);
      end
    end
    setled(s1,-1,black);
    sync(s1);
  end
end

diff={};
window=39;
corder=[1 0 0
        0 1 0
        0 0 1];
cstring='rgbk';
mp={};
for i=1:length(leds)
  setfig(['imdiff',num2str(i)]);
  clf;
  set(gcf,'DefaultAxesColorOrder',corder);
  d1=zeros(size(im{i,1},1),size(im{i,1},2),class(im{i,1}));
  for c=1:length(colors)-1
    d1=d1+sum(im{i,c}-im{i,length(colors)},3);
  end
  [~,maxpt]=max(d1(:));
  [maxpos(1),maxpos(2)]=ind2sub(size(d1),maxpt);
  if maxpos(1)<=window ||  maxpos(1)+window>size(d1,1) || maxpos(2)<=window || maxpos(2)+window>size(d1,2)
    error('Peak for LED %d is closer than %d pixels to edge of ROI', leds(i), window);
  end
  mp{i}=maxpos;
  for c=1:length(colors)-1
    d1=im{i,c}-im{i,length(colors)};
    psf{i,c}=d1(maxpos(1)+(-window:window),maxpos(2)+(-window:window),:);

    subplot(length(colors)-1,5,(c-1)*5+1);
    imshow(psf{i,c}+0.5);
    if c==1
      title(sprintf('%d,%d',maxpos));
    end

    subplot(length(colors)-1,5,(c-1)*5+2);
%    plot(-window:window,squeeze(psf{i,c}(window+1,:,:)))
    plot(-window:window,squeeze(sum(psf{i,c},1)));
    if c==1
      title(sprintf('LED %d, Along x',leds(i)));
    end
    cax=axis;
    axis([-window,window,-1,cax(4)]);

    subplot(length(colors)-1,5,(c-1)*5+4);
%    plot(-window:window,squeeze(psf{i,c}(:,window+1,:)));
    plot(-window:window,squeeze(sum(psf{i,c},2)));
    if c==1
      title('Down along y');
    end
    cax=axis;
    axis([-window,window,-1,cax(4)]);

    us=10;
    sz=size(psf{i,c},1);

    for dir=0:1
      subplot(length(colors)-1,5,(c-1)*5+3+2*dir);
      hold on;
      cutoff=[];
      if rgbsensor
        nplanes=4;
      else
        nplanes=1;
      end
      for tc=1:nplanes
        if tc==4
          mtf=abs(psf2otf(mean(psf{i,c},3),us*sz*[1,1]));
        else
          mtf=abs(psf2otf(psf{i,c}(:,:,tc),us*sz*[1,1]));
        end
        if dir==0
          mtf=mtf(1,:);
        else
          mtf=mtf(:,1);
        end
        mtf=mtf;
        plot((0:us*window-1)/length(mtf),mtf(1:us*window),cstring(tc));
        xlabel('Freq (lp/pixel)');
        cpos=find(mtf(1:us*window)>(0.5*mtf(1)),1,'last');
        if isempty(cpos)
          cutoff(tc)=nan;
        else
          cutoff(tc)=cpos/length(mtf);
        end
      end
      if rgbsensor
        title(sprintf('MTF Cut=(%.1f,%.1f,%.1f,%.1f)',1 ./cutoff));
      else
        title(sprintf('MTF Cut=%.1f',1 ./cutoff));
      end
      ax=axis;
      axis([0 0.5 0 ax(4)]);
      if c==4
        if rgbsensor
          fprintf('LED %d, dir=%d, Cutoff=(%.1f, %.1f, %.1f, %.1f) pixels/lp\n',leds(i),dir,1./cutoff);
        else
          fprintf('LED %d, dir=%d, Cutoff=%.1f pixels/lp\n',leds(i),dir,1./cutoff);
        end
      end
    end
  end
end
% Compute inter-led distance in pixels
delta=(mp{end}-mp{1})/abs(leds(end)-leds(1));
fprintf('Inter-LED distance = (%.2f,%.2f) = %.2f pixels\n', delta, norm(delta));