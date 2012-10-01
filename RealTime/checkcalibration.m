% Display each given LEDs current snapshot along with reference images, correlations
% Useful for debugging odd behaviours of particular LEDs
% Usage: checkcalibration(p,vis,leds,cameras,figname)
%  p - pstruct
%  vis - vis struct as returned by getvisible() or rcvr()
%  leds - list of LED numbers to display (1 or 2 best)
%  camera - list of camera numbers (default: all)
%  figname - arg to setfig to keep a separate figure
function checkcalibration(p,vis,leds,cameras,figname)
rescale=false;  % Rescale images so maximum pixel=1.0
bigwindow=50;   % If set, also show big window
if nargin<3
  nl=4;
  leds=round(1:(numled()-1)/(nl-1):numled());
else
  nl=length(leds);
end

if nargin<2 || isempty(vis)
  % Use lower level to avoid clipping
  vis=getvisible(p,'onval',20*[1 1 1],'stats');
end

if nargin<4
  nc=length(p.camera);
  cameras=1:nc;
else
  nc=length(cameras);
end

if nargin<5
  setfig('checkcalibration');
else
  setfig(figname);
end
clf;
nstack=1;
if isfield(vis,'refim')
  nstack=nstack+1;
end
if exist('bigwindow','var')
  nstack=nstack+1;
end

for ci=1:length(cameras)
  c=cameras(ci);
  pixcalib=p.camera(c).pixcalib;
  vc=p.camera(c).viscache;

  fvalid=find([pixcalib.valid]);
  fprintf('Camera %d, LEDs = %s\n',c,shortlist(fvalid));
  first=1;
  for i=1:nl
    l=leds(i);
    if ~pixcalib(l).valid
      continue;
    end
    stack=1;
    if isfield(vis,'im')
      subplot(nc,nl*nstack,(ci-1)*nl*nstack+(i-1)*nstack+stack);
      stack=stack+1;
      im=vis.im{c}(vc.tlpos(l,2):vc.brpos(l,2),vc.tlpos(l,1):vc.brpos(l,1),:);
      if exist('bigwindow','var')
        bigwin=[max(1,vc.tlpos(l,2)-bigwindow),min(size(vis.im{c},1),vc.brpos(l,2)+bigwindow),max(1,vc.tlpos(l,1)-bigwindow),min(size(vis.im{c},2),vc.brpos(l,1)+bigwindow)];
        bigim=vis.im{c}(bigwin(1):bigwin(2),bigwin(3):bigwin(4),:);
      end
      if rescale
        imsc=im2double(im);
        imsc=imsc/max(imsc(:));
        imshow(imsc);
      else
        imshow(im);
      end
      hold on;
      if first
        ylabel(sprintf('Camera %d',c));
        first=0;
      end
      if pixcalib(l).valid
        title(sprintf('vis.im %d (max=%d)\n', l,max(im(:))));
      else
        title(sprintf('*vis.im %d (max=%d)\n', l, max(im(:))));
      end
      xlabel(sprintf('p=%.3f',vis.corr(c,l)));

      % overlay pixelList boundaries
      plist=pixcalib(l).pixelList;
      roi=p.camera(c).roi;
      plist(:,1)=plist(:,1)-roi(1)+1;
      plist(:,2)=plist(:,2)-roi(3)+1;
      for k=1:size(plist,1)
        plot(plist(k,1)-vc.tlpos(l,1)+[0 1 1]+0.5,plist(k,2)-vc.tlpos(l,2)+[1 1 0]+0.5,'r');
        %        plot(-(rng{1}(1)-1)-0.5+plist(k,1)+[0 1 1 ],-(rng{2}(1)-1)-0.5+plist(k,2)+[1 1 0 ],'r');
      end
    end
    if isfield(vis,'refim')
      subplot(nc,nl*nstack,(ci-1)*nl*nstack+(i-1)*nstack+stack);
      stack=stack+1;
      refim=vis.refim{c}(vc.tlpos(l,2):vc.brpos(l,2),vc.tlpos(l,1):vc.brpos(l,1),:);
      if rescale
        refimsc=refim/max(refim(:));
        imshow(refimsc);
      else
        imshow(refim);
      end
      rrgb=corr2(im(:),refim(:));
      imgray=rgb2graywithweight(im);
      refimgray=rgb2graywithweight(refim);
      % fprintf('TL im=%f,%f,%f (%f), refim=%f,%f,%f (%f)\n', im(1,1,:), imgray(1,1), refim(1,1,:), refimgray(1,1));
      rgray=corr2(imgray,refimgray);
      for j=1:3
        r(j)=corr2(im(:,:,j),refim(:,:,min(size(refim,3),j)));
      end
      title(sprintf('vis.refim %d (max=%.1f)',l,255*max(refim(:))));
      xlabel(sprintf('r=(%.3f,%.3f,%.3f),RGB=%.3f,Gray=%.3f',r,rrgb,rgray));
      ylabel(sprintf('(%d:%d, %d:%d)',vc.tlpos(l,2),vc.brpos(l,2),vc.tlpos(l,1),vc.brpos(l,1)));
    end
    if exist('bigwindow','var')
      subplot(nc,nl*nstack,(ci-1)*nl*nstack+(i-1)*nstack+stack);
      stack=stack+1;
      imshow(bigim);
      title(sprintf('vis.bigim %d',l));
      ylabel(sprintf('(%d:%d, %d:%d)',bigwin));
    end
  end
end
