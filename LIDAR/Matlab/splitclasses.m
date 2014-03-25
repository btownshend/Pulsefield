% Split excessively large classes
function vis=splitclasses(vis,varargin)
defaults=struct('debug',false);
args=processargs(defaults,varargin);
params=getparams();
maxsize=params.maxclasssize;
MAXSPECIAL=2;
jclasses=unique(vis.class(vis.class>MAXSPECIAL));
nextclass=max(jclasses)+1;
redo=true;
while redo
  redo=false;
for j=1:length(jclasses)
  c=jclasses(j);
  p=find(vis.class==c);
  for i=1:length(p)-1
    if norm(vis.xy(p(end),:)-vis.xy(p(i),:)) < maxsize
      break;
    end
  end
  % Distance from i to end is <maxlegdiam, but i-1 to end is >maxlegdiam
  % Break is the last position that is part of leg1
  minbreak=i-1;  
  for i=length(p):-1:1
    if norm(vis.xy(p(1),:)-vis.xy(p(i),:)) < maxsize
      break;
    end
  end
  % Distance from i to 1 is <maxlegdiam, but i+1 to 1 is >maxlegdiam
  maxbreak=i;  
  if minbreak>0
    if minbreak>maxbreak
      if args.debug
        fprintf('Split class %d - too big for one break\n', c);
      end
      minbreak=1;
      maxbreak=length(p)-1;
      redo=true;
    end
    % Target too large, split it at biggest jump
    delta=diff(vis.xy(p(minbreak:maxbreak+1),1)).^2+diff(vis.xy(p(minbreak:maxbreak+1),2)).^2;
    [~,bkpt]=max(delta);
    bkpt=bkpt+minbreak-1;
    vis.class(p(bkpt+1:end))=nextclass;
    nextclass=nextclass+1;
    if vis.range(p(bkpt))>vis.range(p(bkpt+1))
      vis.shadowed(p(bkpt),2)=true;
    else
      vis.shadowed(p(bkpt+1),1)=true;
    end
    if args.debug
      fprintf('Split class %d with diameter %.2f, %d pts, into classes %d,%d at position %d, minbreak=%d, maxbreak=%d\n', c,norm(vis.xy(p(1),:)-vis.xy(p(end),:)),length(p),c,nextclass-1,bkpt,minbreak, maxbreak);
    end
  end
end
end

% Compress class numbers
jclasses=unique(vis.class(vis.class>MAXSPECIAL));
for i=1:length(jclasses)
  vis.class(vis.class==jclasses(i))=i+MAXSPECIAL;
end
