% Update background model
function bg=updatebg(bg,vis,varargin)
defaults=struct('minsep',0.1,'tc',100,'mincntcloser',800,'mincntfarther',40,'debug',false);
args=processargs(defaults,varargin);

if isempty(bg) 
  bg=struct('range',vis.range,'newrange',vis.range,'newcnt',zeros(length(vis.range),1),'angle',vis.angle);
  return;
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
update=((bg.newcnt>args.mincntcloser & bg.newrange(:)<bg.range(:)) | (bg.newcnt>args.mincntfarther & bg.newrange(:)>bg.range(:)));
if sum(update)>0
  if args.debug
    ncloser=sum(update(bg.newrange(:)<bg.range(:)));
    nfarther=sum(update(bg.newrange(:)>bg.range(:)));
    fprintf('** Updating %d closer, %d farther background pixels\n', ncloser, nfarther);
  end
  bg.range(update)=bg.newrange(update);
  bg.newcnt(update)=0;
end

