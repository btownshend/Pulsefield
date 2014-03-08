% Calculate bounding boxes and positions based on current classifications
% Also pack class numbers to eliminate missing classes
function vis=calcbboxes(vis)
MAXSPECIAL=2;

% Calculate centroids and bounding boxes, pack class numbers
pos=[];
bbox=[];
for i=MAXSPECIAL+1:max(vis.class)
  sel=vis.class==i;
  if sum(sel)==0
    continue;
  end
  pos(end+1,:)=mean(vis.xy(sel,:),1);
  bbox(end+1,:)=[min(vis.xy(sel,1)),min(vis.xy(sel,2)),max(vis.xy(sel,1))-min(vis.xy(sel,1)),max(vis.xy(sel,2))-min(vis.xy(sel,2))];
  cnum=size(pos,1)+MAXSPECIAL;
  vis.class(vis.class==i)=cnum;
end
vis.targets=struct('pos',pos,'bbox',bbox);
