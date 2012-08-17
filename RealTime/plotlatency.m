function plotlatency(recvis)

if length(recvis.vis)>length(recvis.snap)
  fprintf('Truncating recvis.vis from %d to %d samples to match recvis.snaps\n', length(recvis.vis), length(recvis.snap));
  recvis.vis=recvis.vis(1:length(recvis.snap));
end

jitter=[];acquired=[];
for i=1:length(recvis.vis)
  acquired(i)=max(recvis.vis(i).acquired);
  jitter(i)=max(recvis.vis(i).acquired)-min(recvis.vis(i).acquired);
end

setfig('latency');
clf;

subplot(511);
plot(1:length(recvis.vis),jitter*24*3600,'g');
c=axis;c(3)=0;axis(c);
title('Jitter (latest-earliest frame)');
xlabel('Sample');
ylabel('Jitter (s)');

subplot(512);
rdelay=([recvis.vis.whenrcvd]-acquired)*24*3600;
% Check for prior frames whose processing time may have delay receipt
waittime=([recvis.vis.whenrcvd]-[nan,recvis.snap(1:end-1).whendone])*3600*24;
latefinish=waittime<1e-4;   % Basically, time was determined by prior processing time
rdelay(latefinish)=nan;
plot(1:length(recvis.vis),rdelay);
if sum(latefinish)>0
  hold on;
  plot(find(latefinish),0*find(latefinish),'*');
end
c=axis;c(3)=0;axis(c);
ylabel('Delay before receipt (s)');
title('Reception delay');

subplot(513);
dur=([recvis.snap.whendone]-[recvis.vis.whenrcvd])*24*3600;
plot(1:length(recvis.snap),dur,'r');
if isfield(recvis.snap,'whendone2')
  hold on;
  dur=([recvis.snap.whendone2]-[recvis.vis.whenrcvd])*24*3600;
  plot(1:length(recvis.snap),dur,'b');
end
c=axis;c(3)=0;c(4)=max(dur(10:end)); axis(c);
ylabel('Processing time (s)');
title('Processing Latency (r=analyze, b=all incl/osc)');

subplot(514);
plot(1:length(recvis.snap),([recvis.snap.whendone]-(acquired-jitter/2))*24*3600,'r');
c=axis;c(3)=0;axis(c);
title('Mean latency (from middle frame to processing done)');
ylabel('Total latency (s)');

subplot(515);
plot(2:length(recvis.snap),diff([recvis.vis.frame])-1,'r');
c=axis;c(3)=0;axis(c);
title('Frame drops');
ylabel('Number of dropped VIS frames');
