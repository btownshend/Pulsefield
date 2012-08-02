% Plot statistics from a recording
function plotstats(snap)
setfig('hypo stats');clf;
time={};speed={};area={};orientation={};majoraxislength={};minoraxislength={};heading={};valid={};
for i=1:length(snap)
  h=snap(i).hypo;
  if ~isempty(h)
    ids=unique([h.id]);
    for jj=1:length(ids)
      j=ids(jj);
      speed{j}(i)=norm(h(jj).velocity);
      heading{j}(i)=h(jj).heading;
      area{j}(i)=h(jj).area;
      orientation{j}(i)=h(jj).orientation;
      time{j}(i)=h(jj).lasttime;
      majoraxislength{j}(i)=h(jj).majoraxislength;
      minoraxislength{j}(i)=h(jj).minoraxislength;
      valid{j}(i)=1;
    end
  end
end
for j=1:length(time)
  speed{j}(~valid{j})=nan;
  heading{j}(~valid{j})=nan;
%  heading{j}=unwrap(heading{j}*pi/180)*180/pi;
  area{j}(~valid{j})=nan;
  orientation{j}(~valid{j})=nan;
  majoraxislength{j}(~valid{j})=nan;
  minoraxislength{j}(~valid{j})=nan;
  time{j}(~valid{j})=nan;
  time{j}=(time{j}-snap(1).when)*24*3600;
  time{j}(time{j}<0)=nan;
end

plotvar(time,speed,'Speed','m/s',[0,nan],1);
plotvar(time,heading,'Heading','deg',[nan,nan],2);
plotvar(time,area,'Area','m',0.3,3);
plotvar(time,minoraxislength,'Minor Axis Length','m',0.8,4);
plotvar(time,majoraxislength,'Major Axis Length','m',0.8,5);
plotvar(time,orientation,'Orientation','deg',[-90,90],6);

function plotvar(t,v,lbl,units,maxval,snum)
if length(maxval)==2
  minval=maxval(1);
  maxval=maxval(2);
else
  minval=0;
end
snum=(snum-1)*3+1;
subplot(6,3,[snum,snum+1]);hold on;
for j=1:length(t)
  if any(isfinite(v{j}))
    plot(t{j},v{j},'Color',id2color(j));
  end
end
xlabel('Time (sec)');
ylabel([lbl,' (',units,')']);
title(lbl);

c=axis;
if isfinite(minval) 
  c(3)=minval; 
end; 
if isfinite(maxval) 
  c(4)=maxval;
end; 
axis(c);

subplot(6,3,snum+2); hold on;
for j=1:length(t)
  if sum(isfinite(v{j}))>20
    h=cdfplot(v{j}(isfinite(v{j})));
    set(h,'Color',id2color(j));
  end
end
c=axis;if isfinite(minval) c(1)=minval;end; if isfinite(maxval) c(2)=maxval;end;axis(c);
xlabel([lbl,' (',units,')']);
title([lbl,' CDF']);
