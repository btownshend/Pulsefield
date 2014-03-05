% Update background model
function bg=updatebg(bg,vis,varargin)
defaults=struct('minsep',0.3,'tc',100,'mincnt',40,'debug',false);
args=processargs(defaults,varargin);

if isempty(bg) 
  bg=struct('range',vis.range,'newrange',vis.range,'newcnt',zeros(1,length(vis.range)),'angle',vis.angle);
  return;
end

farther=vis.range>bg.range+args.minsep;
if sum(farther)>0
  if args.debug
    fprintf('Moving %d background pixels to a more distant target\n', sum(farther));
  end
  bg.range(farther)=vis.range(farther);
  bg.newcnt(farther)=0;
end

isbg=false(1,length(bg.range));
for i=1:length(bg.range)
  isbg(i)=norm(bg.range(i)-vis.range(i))<args.minsep;
  isnewbg(i)=norm(bg.newrange(i)-vis.range(i))<args.minsep;
end
isnewbg=isnewbg&~isbg;

% Averaging for current background
bg.range(isbg)=vis.range(isbg)/args.tc+bg.range(isbg)*(1-1/args.tc);
bg.newcnt(~isnewbg)=0;   % Reset count if we're not close to new background
bg.newrange(bg.newcnt==0)=vis.range(bg.newcnt==0);   % Reset to current position
bg.newcnt(isnewbg)=bg.newcnt(isnewbg)+1;  % Increase count if we are
update=bg.newcnt>args.mincnt;
if sum(update)>0
  if args.debug
    fprintf('Updating %d background pixels\n', sum(update));
  end
  bg.range(update)=bg.newrange(update);
  bg.newcnt(update)=0;
end

