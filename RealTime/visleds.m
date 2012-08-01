% Update LEDS with ray visibility data for debugging
function visleds(p,vis)
v=vis.v;
s1=arduino_ip(0);
setled(s1,[0,numled()-1],p.colors{1}*127,1); 
for c=1:size(v,1)
  indices=find(v(c,:)==0);
  if ~isempty(indices)
    col=p.colors{c+1}*127;
    setled(s1,indices-1,col);
    %fprintf('LEDs %s blocked from camera %d; set to [%d,%d,%d]\n',shortlist(indices),c,col);
  end
end
multiblock=sum(find(v==0),1)>1;
setled(s1,multiblock,p.colors{size(v,1)+2}*127);
show(s1);
