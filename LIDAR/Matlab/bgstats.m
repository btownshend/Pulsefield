% Plot mean, variance of background at each scanline
% Assumes no people in picture
function range=bgstats(snap) 
for i=1:length(snap(1).vis.range)
  range(:,i)=arrayfun(@(z) z.vis.range(i),snap);
end
setfig('bgstats');clf;
subplot(211);
plot(mean(range));
xlabel('Scan line');
ylabel('Mean(range)');

subplot(212);
semilogy(std(range));
xlabel('Scan line');
ylabel('Std(range)');

