function col=id2color(id,colors)
if nargin<2
%  map=lines();
%  col=map(mod(id-1,size(map,1))+1,:);
error('id2color() with 1 arg no longer supported');
else
  if id>0
    col=colors{mod(id-1,length(colors)-1)+2};
  else
    col=colors{1};
  end
end

