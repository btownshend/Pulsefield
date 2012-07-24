function plotlatency(recvis)
setfig('latency');
clf;
plot(0.5:length(recvis.vis)-1,diff([recvis.vis.when])*24*3600);
xlabel('Sample');
ylabel('Latency (s)');
title('Latency');
