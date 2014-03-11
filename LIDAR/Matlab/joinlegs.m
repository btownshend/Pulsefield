% Join separate targets that appear to be 2 legs
function vis=joinlegs(vis,varargin)
defaults=struct('maxlegdiam',0.3,...   % Maximum leg diameter
                'minlegdiam',0.1,...   % Minimum
                'maxlegsep',0.3,...    % Maximum space between 2 legs
                'debug',true...
                );
args=processargs(defaults,varargin);
MAXSPECIAL=2;

classes=unique(vis.class(vis.class>MAXSPECIAL));
toobig=false(size(classes));
for i=1:length(classes)
  c1=classes(i);
  p1=find(vis.class==c1);
  d1=norm(vis.xy(p1(1))-vis.xy(p1(end)));
  if d1>args.maxlegdiam;
    if args.debug
      fprintf('Class %d cannot be a single leg since its diameter = %.2f > %.2f\n',c1,d1,args.maxlegdiam);
    end
    toobig(i)=true;
  end
end

classes=classes(~toobig);
matchup=false(length(classes),length(classes));
for i=1:length(classes)
  c1=classes(i);
  p1=find(vis.class==c1);
  for j=i+1:length(classes)
    c2=classes(j);
    p2=find(vis.class==c2);
    sep=min(norm(vis.xy(p1(1))-vis.xy(p2(end))), norm(vis.xy(p1(end))-vis.xy(p2(1))));
    if sep<args.maxlegsep
      if args.debug
        fprintf('Classes %d and %d are separated by %.2f, so could be 2 legs\n', c1,c2, sep);
      end
      matchup(i,j)=true;
    end
  end
end

% Setup vis.leg - 0 for one blob, 1 for left leg, 2 for right leg
vis.leg=nan(size(vis.class));
vis.leg(vis.class>MAXSPECIAL)=0;

while true
  nmatches=sum(matchup,2);
  makematches=find(nmatches==1,1);
  if isempty(makematches)
    break;
  end
  m1=makematches;
  m2=find(matchup(m1,:));
  fprintf('Matching %d (class %d) and %d (class %d)\n', m1, classes(m1), m2, classes(m2));
  vis.leg(vis.class==classes(m1))=1;
  vis.leg(vis.class==classes(m2))=2;
  vis.class(vis.class==classes(m2))=classes(m1);
  matchup=matchup(nmatches~=1,nmatches~=1);
  classes=classes(nmatches~=1);
end

