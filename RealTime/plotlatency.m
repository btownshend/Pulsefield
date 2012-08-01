function plotlatency(recvis)
setfig('latency');
clf;
plot(1:length(recvis.snap),([recvis.snap.whendone]-[recvis.snap.when])*24*3600,'r');
hold on;
plot(1:length(recvis.vis),([recvis.vis.whenrcvd]-[recvis.vis.when])*24*3600);
jitter=[];
for i=1:length(recvis.vis)
  jitter(i)=(max(recvis.vis(i).acquired)-min(recvis.vis(i).acquired))/2;
end
plot(1:length(recvis.vis),jitter*24*3600,'g');

legend('Processed','Rcvd','Jitter');
xlabel('Sample');
ylabel('Latency (s)');
title('Latency (From mean acquistion time to receipt or analysis done)');
c=axis;
c(3)=0;
axis(c);
