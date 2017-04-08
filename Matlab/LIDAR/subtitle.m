% Output subtitle file using alignment from videoalign
function subtitle(filename,x)
fd=fopen(filename,'w');
skip=10;
for i=1:skip:length(x.t)-skip
  fprintf(fd,'%d\n',i);
  fprintf(fd,'%s --> %s\n', timestr(x.t(i)),timestr(x.t(i+skip)));
  fprintf(fd,'%d\n',x.tmap(i));
  fprintf(fd,'\n');
end

function s=timestr(t)
h=floor(t/3600); t=t-h*3600;
m=floor(t/60); t=t-m*60;
s=floor(t); t=t-s;
ms=round(t*1000);
s=sprintf('%02d:%02d:%02d,%03d',h,m,s,ms);
