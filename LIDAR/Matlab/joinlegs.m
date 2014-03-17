% Join separate targets that appear to be 2 legs
function vis=joinlegs(vis,varargin)
defaults=struct('maxlegdiam',0.3,...   % Maximum leg diameter
                'minlegdiam',0.1,...   % Minimum
                'maxlegsep',0.4,...    % Maximum space between 2 legs
                'divergence',11,...    % Divergence of beam in mrad (expands effective breadth of objects)
                'debug',false...
                );
args=processargs(defaults,varargin);
MAXSPECIAL=2;

classes=unique(vis.class(vis.class>MAXSPECIAL));
toobig=false(size(classes));
for i=1:length(classes)
  c1=classes(i);
  p1=find(vis.class==c1);
  d1=norm(vis.xy(p1(1),:)-vis.xy(p1(end),:));
  maxleg=args.maxlegdiam+vis.range(p1(1))*args.divergence/1000;
  if d1>maxleg
    if args.debug
      fprintf('Class %d cannot be a single leg since its diameter = %.2f > %.2f\n',c1,d1,maxleg);
    end
    toobig(i)=true;
  end
end

classes=classes(~toobig);
matchup=inf(length(classes),length(classes));
for i=1:length(classes)
  c1=classes(i);
  p1=find(vis.class==c1);
  for j=i+1:length(classes)
    c2=classes(j);
    p2=find(vis.class==c2);
    sep=min(norm(vis.xy(p1(1),:)-vis.xy(p2(end),:)), norm(vis.xy(p1(end),:)-vis.xy(p2(1),:)));
    if sep<args.maxlegsep+args.minlegdiam
      if args.debug
        fprintf('Classes %d and %d are separated by %.2f, so could be 2 legs\n', c1,c2, sep);
      end
      matchup(i,j)=sep;
    end
  end
end

% Setup vis.leg - 0 for one blob, 1 for left leg, 2 for right leg
vis.leg=nan(size(vis.class));
vis.leg(vis.class>MAXSPECIAL)=0;

while true
  found=false;
  for m1=1:size(matchup,1)
    for m2=m1+1:size(matchup,2)
      if isfinite(matchup(m1,m2)) && matchup(m1,m2)==min(matchup(m1,:)) && matchup(m1,m2)==min(matchup(:,m2))
        if args.debug
          fprintf('Matching %d (class %d) and %d (class %d) with distance %.2f\n', m1, classes(m1), m2, classes(m2),matchup(m1,m2));
        end
        vis.leg(vis.class==classes(m1))=1;
        vis.leg(vis.class==classes(m2))=2;
        vis.class(vis.class==classes(m2))=classes(m1);
        matchup=matchup([1:m1-1,m1+1:m2-1,m2+1:end],[1:m1-1,m1+1:m2-1,m2+1:end]);
        classes=classes([1:m1-1,m1+1:m2-1,m2+1:end]);
        found=true;
        break;
      end
    end
    if found
      break;
    end
  end
  if ~found
    break;
  end
end

if any(isfinite(matchup(:)))
  fprintf('Warning: still have leg matchups remaining, but ambiquous\n');
  matchup
  keyboard
end

% Now split excessively large legs if possible
jclasses=unique(vis.class(vis.leg==0));
for j=1:length(jclasses)
  c=jclasses(j);
  p=find(vis.class==c);
  for i=1:length(p)-1
    if norm(vis.xy(p(end),:)-vis.xy(p(i),:)) < args.maxlegdiam
      break;
    end
  end
  % Distance from i to end is <maxlegdiam, but i-1 to end is >maxlegdiam
  % Break is the last position that is part of leg1
  minbreak=i-1;  
  for i=length(p):-1:minbreak+1
    if norm(vis.xy(p(1),:)-vis.xy(p(i),:)) < args.maxlegdiam
      break;
    end
  end
  % Distance from i to 1 is <maxlegdiam, but i+1 to end is >maxlegdiam
  maxbreak=i;  
  if minbreak>0
    % Target too large, split it at biggest jump
    delta=diff(vis.xy(p(minbreak:maxbreak),1)).^2+diff(vis.xy(p(minbreak:maxbreak),2)).^2;
    [~,bkpt]=max(delta);
    bkpt=bkpt+minbreak-1;
    vis.leg(p(1:bkpt))=1;
    vis.leg(p(bkpt+1:end))=2;
    if vis.range(p(bkpt))>vis.range(p(bkpt+1))
      vis.shadowed(p(bkpt),2)=true;
    else
      vis.shadowed(p(bkpt+1),1)=true;
    end
    if args.debug
      fprintf('Split class %d into 2 legs at position %d, minbreak=%d, maxbreak=%d\n', c,bkpt,minbreak, maxbreak);
    end
  end
end
