% Plot leftness of an ID
function plotleftness(snap,id)
x=[];y=[];
for i=1:length(snap)
  t=snap(i).tracker;
  if ~isempty(t.tracks)
    sel=[t.tracks.id]==id;
    if sum(sel)>0
      y(end+1)=t.tracks(sel).leftness;
      x(end+1)=snap(i).vis.frame;
    end
  end
end
setfig('leftness');clf;
plot(x,y,'.-');
xlabel('Frame');
ylabel('Leftness');
