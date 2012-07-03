function setwave(s1,index,cols)
cdiff=double(cols(2,:)-cols(1,:))/(index(2)-index(1));
cmd=[];
for i=index(1):index(2)
  col=round(cols(1,:)+cdiff*(i-index(1)));
  cmd=[cmd,setled(s1,i,col,1)];
end
awrite(s1,cmd);
