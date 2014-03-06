% Classify range elements
% 0 - background
% 1-n - target i
function targets=classify(vis,bg,varargin)
defaults=struct('minsep',0.3);
args=processargs(defaults,varargin);
class=[];
for i=1:length(vis.range)
  if norm(vis.range(i)-bg.range(i))<args.minsep
    class(i)=0;
  elseif i>1 && norm(vis.range(i)-bg.range(i-1))<args.minsep
    class(i)=0;
  elseif i<length(vis.range) && norm(vis.range(i)-bg.range(i+1))<args.minsep
    class(i)=0;
  elseif i>1 && norm(vis.range(i)-vis.range(i-1))<args.minsep && class(i-1)~=0
    class(i)=class(i-1);
  else
    class(i)=max([0,class(1:i-1)])+1;
  end
end
pos=[];
bbox=[];
for i=1:max(class)
  sel=class==i;
  xy=range2xy(vis.angle(sel)+pi/2,vis.range(sel));
  pos(i,:)=mean(xy,1);
  bbox(i,:)=[min(xy(:,1)),min(xy(:,2)),max(xy(:,1))-min(xy(:,1)),max(xy(:,2))-min(xy(:,2))];
end
targets=struct('class',class,'pos',pos,'bbox',bbox);
