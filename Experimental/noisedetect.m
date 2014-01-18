% Do background detection of a frame using noise models for each pixel
% Process:
%  - analyze each of R,G,B independently
%   - compute distribution for each pixel (mean,var) when background present from vis.refim and vis.refim2
%   - normalize current image (vis.im) by subtracting means, squaring, divide by var =>  (number of stds from mean for each pixel)^2
%   - average over rectangular windows given by wsize (or p.camera.viscache.wsize)
%   - count fraction within each window that are <fgmaxvar
%  - average the fraction over R,G,B
%  - compare with (1-vis.corr)*fgscale (should be the same as C++ program does the same computation)
%  - sample at center points of each window in p.camera().viscache.{tl,br}pos
%  - decide if background if fracgood>0.5
%  - compare with decision encoded in vis
function noisedetect(p,vis,cam,varargin)
defaults=struct('wsize',[]);
args=processargs(defaults,varargin);

if isempty(args.wsize)
  args.wsize= p.camera(1).viscache.wsize;
end

setfig('noisedet');clf;
nrow=6; ncol=1; pnum=1;

nm=noisemodel(p,vis,cam);
img=im2double(vis.im{cam});
diff=img-nm.avg;
nvar=diff.*diff./nm.var;
fprintf('Mean sample level: %.1f/255, mean std: %.1f/255\n',255*mean(nm.avg(:)),255*mean(sqrt(nm.var(:))));
fprintf('Fraction >1 std from avg: %.3f\n',mean(nvar(:)>1));
fprintf('Fraction >2 std from avg: %.3f\n',mean(nvar(:)>2^2));

subplot(nrow,ncol,pnum); pnum=pnum+1;
vscale=10;
imshow(sqrt(nm.var)*255/vscale);
title(sprintf('Pixel STD (in 255-scale) / %d', vscale));

subplot(nrow,ncol,pnum); pnum=pnum+1;
ufscale=10;
imshow(nvar/ufscale);
title(sprintf('Unfiltered deviation / %.1f',ufscale));

thresh=p.analysisparams.fgthresh(2);

%subplot(nrow,ncol,pnum); pnum=pnum+1;
%hist(min(thresh,nvar(:)),1000);
%title('Distribution of nvar');

fprintf('Using window size of [%d,%d]\n',args.wsize);
H=fspecial('average',args.wsize);
%H=fspecial('gaussian',args.wsize,5);
if 0
  % Use maximum
  fc=[];
  for c=1:3
    fc(:,:,c)=nlfilter(nvar(:,:,c),args.wsize,@(x) max(x(:)) );
  end
  f=sqrt(max(fc,[],3));
else
  f=sqrt(mean(imfilter(nvar,H),3));
end
fracgood=mean(imfilter(nm.var<=p.analysisparams.fgmaxvar,fspecial('average',args.wsize),'replicate'),3);
fprintf('Have %d (%.1f%%) pixels with variance > (%.1f/255)^2\n', sum(fracgood(:)<0.5),mean(fracgood(:)<0.5)*100,sqrt(p.analysisparams.fgmaxvar)*255);

subplot(nrow,ncol,pnum); pnum=pnum+1;
imshow(f/thresh);
title(sprintf('Filtered deviation / %.2f',thresh));

for i=1:2
  thresh=p.analysisparams.fgthresh(i);
  det=f>thresh;
  rgb=img;
  rgb(:,:,2)=max(rgb(:,:,2),det);
  rgb(:,:,1)=max(rgb(:,:,1),fracgood<0.5);
  subplot(nrow,ncol,pnum); pnum=pnum+1;
  imshow(rgb);
  title(sprintf('Deviation>%.2f',thresh));
end

%subplot(nrow,ncol,pnum); pnum=pnum+1;
%hist(f(:),1000);
%title('Distribution of f');

% Overall plot with LED decisions overlaid
subplot(nrow,ncol,pnum); pnum=pnum+1;
%imshow(nvar/max(nvar(:)));
imshow(f/thresh);
hold on;
pc=p.camera(cam).pixcalib;
vc=p.camera(cam).viscache;
roi=p.camera(cam).roi([1,3]);
nled=size(vis.corr,2);
for l=1:nled
  pl=round((vc.tlpos(l,:)+vc.brpos(l,:))/2);
  nstd=(1-vis.corr(cam,l))*p.analysisparams.fgscale;
  if ~any(isnan(pl))
    pf(l)=f(pl(2),pl(1));
    isgood=fracgood(pl(2),pl(1))>0.5;
    if ~isgood
      fprintf('LED %d not used since %.2f fraction of the pixels have variance > %.2f\n', l, fracgood(pl(2),pl(1)),p.analysisparams.fgmaxvar);
      plot(pl(1),pl(2),'m');
    elseif nstd<p.analysisparams.fgthresh(1)
      plot(pl(1),pl(2),'g');
      if vis.v(cam,l)~=1
        fprintf('LED %d has vis.vorig=%d, but should be 1\n',l,vis.vorig(cam,l));
      end
    elseif isnan(nstd) || nstd<p.analysisparams.fgthresh(2)
      plot(pl(1),pl(2),'y');
      if ~isnan(vis.v(cam,l))
        fprintf('LED %d has vis.vorig=%d, but should be NaN\n',l,vis.vorig(cam,l));
      end
    else
      plot(pl(1),pl(2),'r');
      if vis.v(cam,l)~=0
        fprintf('LED %d has vis.vorig=%d, but should be 0\n',l,vis.vorig(cam,l));
      end
    end
  else
    pf(l)=nan;
  end
end

% subplot(nrow,ncol,pnum); pnum=pnum+1;
% fecorr=vis.corr(cam,:);
% recorr=1-pf/p.analysisparams.fgscale;
% plot(fecorr,'m');
% hold on;
% plot(recorr);
% plot([1,nled],[vis.mincorr,vis.mincorr],':');
% legend('From FrontEnd','Recomputed');
% title(sprintf('FrontEnd corr (mean=%.3f) and reccomputed (%.3f)', nanmean(fecorr), nanmean(recorr)));

setfig('noisedet2');clf;
festd=(1-vis.corr(cam,:))*p.analysisparams.fgscale;
plot(festd,'m');
hold on;
plot(pf);
plot([1,nled],p.analysisparams.fgthresh(1)*[1,1],':');
plot([1,nled],p.analysisparams.fgthresh(2)*[1,1],':');
legend('From FrontEnd','Recomputed');
title(sprintf('FrontEnd nstd (mean=%.3f) and reccomputed (%.3f)', nanmean(festd), nanmean(pf)));

