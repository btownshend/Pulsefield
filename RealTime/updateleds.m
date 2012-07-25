% Update MAX with a new set of locations for the targets
function updateleds(p,layout,snap)
% Send new values
s1=arduino_ip(0);
mindist=1.0;   % 1m
distrange=0.02;  % If within this distance of min, then change color
setled(s1,[0,numled()-1],p.colors{1},1); 
for i=1:length(snap.hypo)
  h=snap.hypo(i);
  pos=h.pos;
  col=id2color(h.id);
  diam=h.majoraxislength;
  d2=sqrt((layout.lpos(:,1)-pos(1)).^2+(layout.lpos(:,2)-pos(2)).^2);
  [dmin,closest]=min(d2);
  %fprintf('Mindist to target %d = %.1f\n', i, dmin);
  if dmin<=mindist
    indices=find(d2<dmin+distrange)-1;
    %indices=closest;   % Just closest one for testing
    setled(s1,indices,col);
    % fprintf('setled(%s,[%d,%d,%d])\n',shortlist(indices),col);
  end
  % Visual feedback of how many people are inside
  setled(s1,i-1,col,1);
end
show(s1); 
