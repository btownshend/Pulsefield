function col=id2color(id)
map=lines();
col=map(mod(id-1,size(map,1))+1,:);

