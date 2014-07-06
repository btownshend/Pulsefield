% Plot a particular bg scanline
function plotbg(snap,scanline)
frame=arrayfun(@(z) z.vis.frame,snap);
vrange=arrayfun(@(z) z.vis.range(scanline),snap);

setfig('plotbg');clf;
subplot(311);
plot(frame,vrange,'g');
hold on;
col='rbm';
for i=1:3
  range=arrayfun(@(z) z.bg.range(i,scanline),snap);
  plot(frame+i/4,range,col(i));
end
legend('Vis','Bg1','Bg2','Bg3');
xlabel('Frame');
ylabel('Range');

subplot(312);
for i=1:3
  sigma=arrayfun(@(z) z.bg.sigma(i,scanline),snap);
  plot(frame+i/4,sigma,col(i));
  hold on;
end
legend('Bg1','Bg2','Bg3');
xlabel('Frame');
ylabel('Sigma');

subplot(313);
freq=[];
for i=1:3
  freq(i,:)=arrayfun(@(z) z.bg.freq(i,scanline),snap);
end
h=bar(frame,freq','stacked');
for i=1:3
  set(h(i),'FaceColor',col(i));
  set(h(i),'EdgeColor',col(i));
end
xlabel('Frame');
ylabel('Freq');
c=axis;
c(3)=0;
c(4)=1;
axis(c);
