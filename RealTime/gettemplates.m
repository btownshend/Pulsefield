% gettemplates - template image for each LED
function [p,sc]=gettemplates(p)
if ~isfield(p.camera(1).pixcalib(1),'ref')
  % Put in dummy templates so getvisible can be callled
  dummy=zeros(2*p.analysisparams.bbwind+1,2*p.analysisparams.bbwind+1);
  for c=1:length(p.camera)
    for l=1:length(p.led)
      p.camera(c).pixcalib(l).ref=dummy;
    end
  end
end

% Get images
nsamp=8;
for i=1:nsamp
  vis{i}=getvisible(p,1);
end

% Copy in true template
sc=[];
for c=1:length(p.camera)
  for l=1:length(p.led)
    if p.camera(c).pixcalib(l).valid
      s=vis{1}.tgt{c,l};
      for i=2:nsamp
        s=s+vis{i}.tgt{c,l};
      end
      s=s/nsamp;s=s-mean(s(:));s=s/std(s(:));
      % Check correlation of each image with avg
      for i=1:nsamp
        sc(c,l,i)=corr2(vis{i}.tgt{c,l},s);
      end
      if min(sc(c,l,:))<0.8
        fprintf('Warning: corr(%d,%d,:)=%.2f - disabling use\n', c,l,min(sc(c,l,:)));
        p.camera(c).pixcalib(l).inuse=false;
      else
        p.camera(c).pixcalib(l).inuse=true;
      end
      p.camera(c).pixcalib(l).ref=s;
    end
  end
end