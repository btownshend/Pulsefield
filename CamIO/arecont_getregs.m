% Get all register values from arecont
function r=arecont_getregs(id)
[h,p]=getsubsysaddr(sprintf('CA%d',id));
r=[];
for page=0:6
  fprintf('Getting page %d\n',page);
  for reg=0:255
    url=sprintf('http://%s/getreg?page=%d&reg=%d',h,page,reg);
    [resp,status]=urlread(url);
    if status~=1
      error('Failed read of %s',url);
    end
    [x,cnt]=sscanf(resp,'Reg[%d:%d]=%d');
    if cnt~=3
      error('Only matched %d fields',cnt);
    end
    r(page+1,reg+1)=x(3);
  end
end