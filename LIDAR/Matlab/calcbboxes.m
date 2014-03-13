% Calculate bounding boxes and positions based on current classifications
% Also pack class numbers to eliminate missing classes
function vis=calcbboxes(vis)
MAXSPECIAL=2;

% Calculate centroids and bounding boxes, pack class numbers
pos=[];
bounded=[];
legs=[];
for i=MAXSPECIAL+1:max(vis.class)
  sel=vis.class==i;
  if sum(sel)==0
    continue;
  end

  lm=legmodel(vis,i);
  legs=[legs,lm];
  if all(isfinite(lm.c2))
    pos(end+1,:)=(lm.c1+lm.c2)/2;
  else
    pos(end+1,:)=lm.c1;
  end
  fsel=find(sel);
  bounded(end+1)=~vis.shadowed(fsel(1)) && ~vis.shadowed(fsel(end));
  cnum=size(pos,1)+MAXSPECIAL;
  vis.class(vis.class==i)=cnum;
end
vis.targets=struct('legs',legs,'pos',pos,'bounded',bounded);
