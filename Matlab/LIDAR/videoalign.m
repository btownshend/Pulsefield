function x=videoalign(ferecfile, reftrackerframe, refvideoframe, videoFPS)
if nargin<4
  videoFPS=23.976;
end

cmd=sprintf('zgrep "^1 " "%s" | cut "-d " -f 2,3,4 >/tmp/map.txt',ferecfile);
[s,r]=system(cmd);
if s~=0
  error('Failed "%s": %s', cmd,r);
end
map=load('/tmp/map.txt');
t=map(:,2)+map(:,3)/1e6;
tframe=map(:,1);
refindex=find(tframe==reftrackerframe,1);
if isempty(refindex)
  error('Reference tracker frame %d not found in file, which covers frames %d-%d\n', reftrackerframe, tframe([1,end]));
end
vframe=(t-t(refindex))*videoFPS+refvideoframe;
first=find(vframe>=1,1);
last=length(vframe);
fprintf('Mapping tracker frames %d-%d to video frames %.0f-%.0f over a period of %.1f seconds\n', tframe([first,last]), vframe([first,last]),(t(last)-t(first)));
%vmap=round(vframe(first)):round(vframe(last));
%tmap=interp1(vframe,tframe,vmap);
tv1=interp1(vframe,t,1);
x=struct('vmap',round(vframe(first:last)),'tmap',round(tframe(first:last)),'t',t(first:last)-tv1);


