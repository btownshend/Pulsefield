% Split excessively large classes
function vis=splitclasses(vis,varargin)
defaults=struct('debug',false);
args=processargs(defaults,varargin);
params=getparams();
maxsize=params.maxclasssize;
MAXSPECIAL=2;
nextclass=max(vis.class)+1;
c=MAXSPECIAL+1;
while c<nextclass
  p=find(vis.class==c);
  maxdelta=0;
  bkpt=1;
  for i=2:length(p)
    delta=norm(vis.xy(p(i-1),:)-vis.xy(p(i),:));
    if delta>maxdelta
      maxdelta=delta;
      bkpt=i;
    end
    if norm(vis.xy(p(1),:)-vis.xy(p(i),:)) > maxsize
      % Too big, split it
      vis.class(p(bkpt:end))=nextclass;
      nextclass=nextclass+1;
      if p(bkpt-1)==p(bkpt)-1
        % Migh have changed shadowing
        if vis.range(p(bkpt-1))>vis.range(p(bkpt))
          vis.shadowed(p(bkpt-1),2)=true;
        else
          vis.shadowed(p(bkpt),1)=true;
        end
      end
      if args.debug
        fprintf('Split class %d with diameter %.2f, %d pts, into classes %d,%d at position %d\n', c,norm(vis.xy(p(1),:)-vis.xy(p(end),:)),length(p),c,nextclass-1,bkpt);
      end
      break;
    end
  end
  c=c+1;
end

% Compress class numbers
jclasses=unique(vis.class(vis.class>MAXSPECIAL));
for i=1:length(jclasses)
  vis.class(vis.class==jclasses(i))=i+MAXSPECIAL;
end
