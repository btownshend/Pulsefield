% Convert an RGB color value to the nearest TouchOSC gui color
function colname=col2touchosc(col)
colnames={'red'  ,'green','blue' ,'yellow','purple','gray' ,'orange' ,'brown'          ,'pink'   };
colvals= {[1 0 0],[0 1 0],[0 0 1],[1 1 0] ,[1 0 1] ,[1 1 1],[1 0.5 0], [133 102 68]/255,[1 0.5 1]};
cor=[];
for i=1:length(colvals)
  cor(i)=dot(colvals{i},col)/sqrt(dot(colvals{i},colvals{i})*dot(col,col));
end
[mx,mxpos]=max(cor);
colname=colnames{mxpos};