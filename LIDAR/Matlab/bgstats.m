% Plot mean, variance of background at each scanline
% Assumes no people in picture
function range=bgstats(snap) 
for i=1:length(snap(1).vis.range)
  range(:,i)=arrayfun(@(z) z.vis.range(i),snap);
end

setfig('bgstats');clf;
subplot(311);
plot(snap(end).bg.range(1,:),'r');
hold on;
v=snap(end).bg.range(2,:);
sel=snap(end).bg.freq(2,:)>.001;
v(~sel)=nan;
plot(v,'b');
plot(mean(range,1),'k');
xlabel('Scan line');
ylabel('Mean(range)');
legend('Bg1','Bg2','Vis');

subplot(312);
plot(snap(end).bg.freq(1,:)*100,'r');
hold on;
plot(snap(end).bg.freq(2,:)*100,'b');
xlabel('Scan line');
ylabel('Freq (%)');
legend('Bg1','Bg2');

subplot(313);
semilogy(snap(end).bg.sigma(1,:),'r');
hold on;
semilogy(snap(end).bg.sigma(2,:),'b');
semilogy(std(range,1),'k');
xlabel('Scan line');
ylabel('Std(range)');

keyboard

