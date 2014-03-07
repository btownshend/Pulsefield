% Classify range elements
% 0 - background
% 1 - outside of background
% 2 - target too small
% 3-n - target i
function targets=classify(vis,bg,varargin)
defaults=struct('minsep',0.3,'mintarget',0.15);
args=processargs(defaults,varargin);
maxspecial=2;   % Special classes, starting from 0
class=[];
for i=1:length(vis.range)
  if norm(vis.range(i)-bg.range(i))<args.minsep
    class(i)=0;
  elseif vis.range(i)>bg.range(i)
    class(i)=1;
  elseif i>1 && norm(vis.range(i)-bg.range(i-1))<args.minsep
    class(i)=0;
  elseif i<length(vis.range) && norm(vis.range(i)-bg.range(i+1))<args.minsep
    class(i)=0;
  elseif i>1 && norm(vis.range(i)-vis.range(i-1))<args.minsep && class(i-1)>2
    class(i)=class(i-1);
  else
    class(i)=max([maxspecial,class(1:i-1)])+1;
  end
end
pos=[];
bbox=[];
isnoise=false(1,max(class)-maxspecial);
for i=maxspecial+1:max(class)
  sel=class==i;
  if sum(sel)==0
    continue;
  end
  xy=range2xy(vis.angle(sel)+pi/2,vis.range(sel));
  pos(i-maxspecial,:)=mean(xy,1);
  bbox(i-maxspecial,:)=[min(xy(:,1)),min(xy(:,2)),max(xy(:,1))-min(xy(:,1)),max(xy(:,2))-min(xy(:,2))];
  radius=norm(bbox(i-maxspecial,3:4));
  if 0 && radius<args.mintarget
    % Remove 'noise'
    isnoise(i-maxspecial)=true;
  end
end
pos=pos(~isnoise,:);
bbox=bbox(~isnoise,:);
class(ismember(class,maxspecial+find(isnoise)))=2;
targets=struct('class',class,'pos',pos,'bbox',bbox);
