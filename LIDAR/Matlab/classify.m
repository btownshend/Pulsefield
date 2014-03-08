% Classify range elements
% 0 - background
% 1 - outside of background
% 2 - target too small
% 3-n - target i
function targets=classify(vis,bg,varargin)
defaults=struct('maxtgtsep',0.3,...   % Max separation between points that are still the same target
                'maxbgsep',0.1,...    % Max distance from background to be considered part of background
                'mintarget',0.05,...  % Minimum target size (otherwise is noise)
                'minrange',0.1,...    % Minimum range, less than this becomes noise (dirt on sensor glass)
                'debug',false...
                );
args=processargs(defaults,varargin);

% Special classes, starting from 0
BACKGROUND=0;
OUTSIDE=1;
NOISE=2;
MAXSPECIAL=2;

xy=range2xy(vis.angle+pi/2,vis.range);

class=[];
for i=1:length(vis.range)
  if norm(vis.range(i)-bg.range(i))<args.maxbgsep
    class(i)=BACKGROUND;
  elseif vis.range(i)>bg.range(i)
    class(i)=OUTSIDE;
  % elseif i>1 && norm(vis.range(i)-bg.range(i-1))<args.maxbgsep
  %   class(i)=BACKGROUND;
  % elseif i<length(vis.range) && norm(vis.range(i)-bg.range(i+1))<args.maxbgsep
  %   class(i)=BACKGROUND;
  elseif vis.range(i)<args.minrange
    class(i)=NOISE;
  else
    % A target
    % Assume it is a new target first
    class(i)=max([MAXSPECIAL,class(1:i-1)])+1;

    % Check if it is close enough to a prior target
    for j=i-1:-1:1
      if norm(xy(i,:)-xy(j,:))<args.maxtgtsep
        % Close enough
        class(i)=class(j);
        break;
      end
      if vis.range(j)>vis.range(i)
        % A discontinuity which is not shadowed, stop searching
        break;
      end
    end
  end
end

if args.debug
  fprintf('Classify: %d background, %d outside, %d noise, %d targets in %d classes\n', sum(class==0), sum(class==1), sum(class==2), sum(class>2), max(class)-MAXSPECIAL);
end
% Remove noise
for i=MAXSPECIAL+1:max(class)
  sel=class==i;
  if sum(sel)==0
    continue;
  end
  fsel=find(sel);
  sz=norm(xy(fsel(1),:)-xy(fsel(end),:));
  if sz<args.mintarget
    if fsel(1)>1 && vis.range(fsel(1))>vis.range(fsel(1)-1) && class(fsel(1)-1)>MAXSPECIAL
      % Shadowed on the left, may not be noise
      fprintf('Class %d (%d:%d) may be a target shadowed on the left\n', i, min(fsel),max(fsel));
      continue;
    end
    if fsel(end)<length(vis.range) && vis.range(fsel(end))>vis.range(fsel(end)+1) && class(fsel(end)+1)>MAXSPECIAL
      fprintf('Class %d (%d:%d) may be a target shadowed on the right\n', i, min(fsel),max(fsel));
      continue;
    end
    % Remove 'noise'
    fprintf('Target class %d (%d:%d) at (%.1f,%.1f):(%.1f,%.1f) is probably noise\n', i, min(fsel), max(fsel), xy(fsel(1),:),xy(fsel(end),:));
    class(class==i)=NOISE;
  end
end

% Calculate centroids and bounding boxes, pack class numbers
pos=[];
bbox=[];
for i=MAXSPECIAL+1:max(class)
  sel=class==i;
  if sum(sel)==0
    continue;
  end
  pos(end+1,:)=mean(xy(sel,:),1);
  bbox(end+1,:)=[min(xy(sel,1)),min(xy(sel,2)),max(xy(sel,1))-min(xy(sel,1)),max(xy(sel,2))-min(xy(sel,2))];
  cnum=size(pos,1)+MAXSPECIAL;
  class(class==i)=cnum;
end

targets=struct('class',class,'pos',pos,'bbox',bbox);
