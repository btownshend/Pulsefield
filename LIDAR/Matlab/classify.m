% Classify range elements
% 0 - background
% 1 - outside of background
% 2 - target too small
% 3-n - target i
function vis=classify(vis,bg,varargin)
defaults=struct('debug',false);
args=processargs(defaults,varargin);
params=getparams();

% Special classes, starting from 0
BACKGROUND=0;
OUTSIDE=1;
NOISE=2;
MAXSPECIAL=2;

xy=range2xy(vis.angle,vis.range);

class=nan(length(vis.range),1);
shadowed=false(length(vis.range),2);
isbg=bg.isbg(vis);
class(isbg)=BACKGROUND;
%isoutside=vis.range(1,1,:)>=params.maxrange;isoutside=isoutside(:)';
%class(isoutside&~isbg)=OUTSIDE;
%notbg=find(~isbg&~isoutside);
notbg=find(~isbg);
for ii=1:length(notbg)
  i=notbg(ii);
  if vis.range(i)<params.minrange
    class(i)=NOISE;
  else
    % A target
    % Assume it is a new target first
    class(i)=max([MAXSPECIAL;class(1:i-1)])+1;

    % Check if it is close enough to a prior target
    for j=i-1:-1:1
      if norm(xy(i,:)-xy(j,:))<params.maxtgtsep && class(j)>MAXSPECIAL
        % Close enough
        class(i)=class(j);
        break;
      end
      % Note that background calculation spreads out background assignment 1 scan wider than a shadowing background
      % So even if there is no shadow, if the pixel is a background, it could be shadowing
      if vis.range(j)>vis.range(i) % & class(j)~=BACKGROUND
        % A discontinuity which is not shadowed, stop searching
        break;
      end
    end
  end
end

for ii=1:length(notbg)
  i=notbg(ii);
  % Check if it is an isolated point (noise)
  if (i==1 || abs(vis.range(i-1)-vis.range(i))>params.maxtgtsep) && ...
     (i==length(vis.range) || abs(vis.range(i+1)-vis.range(i))>params.maxtgtsep)
    if args.debug
      fprintf('Marking point %d as noise\n', i);
    end
    class(i)=NOISE;
  end
  % See if a point would be shadowed on the left or right
  if i>1 && vis.range(i-1)<vis.range(i) && class(i-1)~=class(i)
    shadowed(i,1)=true;
  end
  if i<length(vis.range) && vis.range(i+1)<vis.range(i) && class(i+1)~=class(i)
    shadowed(i,2)=true;
  end
end

if args.debug
  fprintf('Classify: %d background, %d outside, %d noise, %d targets in %d classes\n', sum(class==BACKGROUND), sum(class==OUTSIDE), sum(class==NOISE), sum(class>MAXSPECIAL), max(class)-MAXSPECIAL);
end
% Remove noise
for i=MAXSPECIAL+1:max(class)
  sel=class==i;
  if sum(sel)==0
    continue;
  end
  fsel=find(sel);
  scanwidth=mean(vis.range(fsel([1,end])))*(vis.angle(2)-vis.angle(1));
  sz=norm(xy(fsel(1),:)-xy(fsel(end),:))+scanwidth;
  if sz<params.mintarget
    if shadowed(fsel(1),1)
      % Shadowed on the left, may not be noise
      if args.debug
        fprintf('Class %d (%d:%d) may be a target shadowed on the left\n', i, min(fsel),max(fsel));
      end
      continue;
    end
    if shadowed(fsel(end),2)
      if args.debug
        fprintf('Class %d (%d:%d) may be a target shadowed on the right\n', i, min(fsel),max(fsel));
      end
      continue;
    end
    % Remove 'noise'
    if args.debug
      fprintf('Target class %d (%d:%d) at (%.1f,%.1f):(%.1f,%.1f) has size %.2f<%.2f, is probably noise\n', i, min(fsel), max(fsel), xy(fsel(1),:),xy(fsel(end),:),sz,params.mintarget);
    end
    class(class==i)=NOISE;
  elseif args.debug
    fprintf('Target class %d (%d:%d) at (%.1f,%.1f):(%.1f,%.1f) spanning %d points has size %.2f>%.2f: accepted\n', i, min(fsel), max(fsel), xy(fsel(1),:),xy(fsel(end),:),length(fsel),sz,params.mintarget);
  end
end

vis.class=class;
vis.shadowed=shadowed;
vis.xy=xy;
