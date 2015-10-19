frame=1;
addpath ../Common
addpath ../RealTime
window=[-0.17 0.17 0 4.0];
dest={'LASER','REC'};
oscmsgout(dest,'/pf/set/minx',{window(1)});
oscmsgout(dest,'/pf/set/maxx',{window(2)});
oscmsgout(dest,'/pf/set/miny',{window(3)});
oscmsgout(dest,'/pf/set/maxy',{window(4)});
for i=1:3
col=[0,0,0];
col(i)=1.0;
while max(col)>0
  oscmsgout(dest,'/pf/frame',{frame});
  frame=frame+1;
  theta=theta+dtheta;
  oscmsgout(dest,'/laser/bg/begin',{});
  for j=1:10
  oscmsgout(dest,'/laser/set/color',{col(1),0.0,0.0});
  oscmsgout(dest,'/laser/circle',{-0.1,2.0,0.05});
  oscmsgout(dest,'/laser/set/color',{0.0,col(2),0.0});
  oscmsgout(dest,'/laser/circle',{0.0,2.0,0.05});
  oscmsgout(dest,'/laser/set/color',{0.0,0.0,col(3)});
  oscmsgout(dest,'/laser/circle',{0.1,2.0,0.05});
  end
  oscmsgout(dest,'/laser/bg/end',{});
  fprintf('%.2f %.2f %.2f\n', col);
  col(col>0)=col(col>0)-0.05;
  x=input('Col? ');
  if ~isempty(x)
    if length(x)==3
      col=x
    else
      col(col>0)=x;
    end
  end
end
end
