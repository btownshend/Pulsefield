% Plot mean, variance of background at each scanline
% Assumes no people in picture
function range=bgstats(snap) 
for i=1:length(snap(1).vis.range)
  range(:,i)=arrayfun(@(z) z.vis.range(i),snap);
end
ol=zeros(1,size(range,2));
olv=ones(1,size(range,2))*0.01;
for i=1:size(range,1)
  [ol,olv]=bgupdate(ol,olv,range(i,:));
end

setfig('bgstats');clf;
subplot(211);
plot(snap(end).bg.range(1,:),'r');
hold on;
v=snap(end).bg.range(2,:);
sel=snap(end).bg.freq(2,:)>.001;
v(~sel)=nan;
plot(v,'b');
plot(mean(range),'k');
plot(ol,'g');
xlabel('Scan line');
ylabel('Mean(range)');
legend('Bg1','Bg2','Vis');

subplot(212);
semilogy(snap(end).bg.sigma(1,:),'r');
hold on;
semilogy(snap(end).bg.sigma(2,:),'b');
semilogy(std(range),'k');
semilogy(sqrt(olv),'g');
xlabel('Scan line');
ylabel('Std(range)');

keyboard

function [ol,olv]=bgupdate(ol,olv,range)
tc=50;
oldol=ol;
ol=ol*(1-1/tc)+range*(1/tc);
olv=olv*(1-1/tc)+(range-ol).*(range-oldol)*(1/tc);
