% setfig - Create a new figure or reuse one with given name
function setfig(s)
global figlist;
undef=0;
eval('figlist.name;','undef=1;');
if undef==1
  figlist.name={s};
  figlist.fignum=20;
  figure(figlist.fignum);
  return;
end
for i=1:length(figlist.name)
  if strcmp(figlist.name(i),s) == 1
    figure(figlist.fignum(i));
    return;
  end
end
figlist.name(length(figlist.name)+1)=cellstr(s);
figure(max(figlist.fignum)+1);
figlist.fignum=[figlist.fignum,gcf];
return;

