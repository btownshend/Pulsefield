% Determine how probability of bg/fg varies with diversity (variance of pixels in a window)
function bgprob(p,vis,cam)
pc=p.camera(cam).pixcalib;
vc=p.camera(cam).viscache;
wsize=p.camera(cam).viscache.wsize;
nled=size(vis.corr,2);
nrandom=1000;
img=im2single(vis.im{cam});
nm=noisemodel(p,vis,cam);
for l=1:nled
  if any(isnan(vc.tlpos(l,:)))
    continue;
  end
  currim=img(vc.tlpos(l,2):vc.brpos(l,2),vc.tlpos(l,1):vc.brpos(l,1),:);
  avg=nm.avg(vc.tlpos(l,2):vc.brpos(l,2),vc.tlpos(l,1):vc.brpos(l,1),:);
  var=nm.var(vc.tlpos(l,2):vc.brpos(l,2),vc.tlpos(l,1):vc.brpos(l,1),:);
  nvar=(currim-avg).^2./var;
  nstd=sqrt(mean(nvar(:)));
  rnstd=[];
  for i=1:nrandom
    while true
      x=randi(size(img,2)-wsize(2)+1,1);
      y=randi(size(img,1)-wsize(1)+1,1);
      % Only use parts at least 1/4 image height and width away to ensure we're not looking at the same structure
      if (abs(x-vc.tlpos(l,1))>size(img,2)/4 && abs(y-vc.tlpos(l,2))>size(img,1)/4)
        break;
      end
    end
    rimg=img(y+(0:wsize(1)-1),x+(0:wsize(2)-1),:);
    rnvar=(rimg-avg).^2 ./ var;
    rnstd(i)=sqrt(mean(rnvar(:)));
  end
  diversity(l)=mean([std(avg(:,:,1)),std(avg(:,:,2)),std(avg(:,:,3))]);
  fracbad(l)=mean(rnstd<=p.analysisparams.fgthresh(2));
  if fracbad(l)>0.01
    fprintf('LED %d: vorig=%d, nstd(correct)=%.2f, diversity=%.3f, mean(nstd)=%.1f, std(nstd)=%.1f, frac<%.1f = %.1f%%\n', l, vis.vorig(cam,l),nstd, diversity(l), mean(rnstd), std(rnstd), p.analysisparams.fgthresh(2), fracbad(l)*100);
  end
end
setfig('bgprob');clf;
plot(diversity,fracbad,'.');
xlabel('Diversity');
ylabel('Fraction matching random part of image');
keyboard

