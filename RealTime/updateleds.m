% Update MAX with a new set of locations for the targets
function updateleds(p,layout,snap)
% Send new values
s1=arduino_ip(0);
mindist=1.0;   % 1m
distrange=0.05;  % If within this distance of min, then change color
setled(s1,[0,numled()-1],p.colors{1},1); 
for i=1:length(snap.hypo)
  pos=snap.hypo(i).pos;
  d2=sqrt((layout.lpos(:,1)-pos(1)).^2+(layout.lpos(:,2)-pos(2)).^2);
  [dmin,closest]=min(d2);
  %fprintf('Mindist to target %d = %.1f\n', i, dmin);
  if dmin<=mindist
    indices=find(d2<dmin+distrange)-1;
    col=p.colors{mod(i-1,length(p.colors)-1)+2};
    setled(s1,indices,col);
    % fprintf('setled(%s,[%d,%d,%d])\n',shortlist(indices),col);
  end
end
show(s1); 
