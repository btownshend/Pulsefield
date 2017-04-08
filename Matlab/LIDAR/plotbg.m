% Plot a particular bg scanline
function plotbg(snap,scanline)
frame=arrayfun(@(z) z.vis.frame,snap);
vrange=arrayfun(@(z) z.vis.range(scanline),snap);

setfig('plotbg');clf;
subplot(311);
plot(frame,vrange,'g');
hold on;
col='rbcmky';
leg={'vis'};
nbg=size(snap(1).bg.range,1)
for i=1:nbg
  range=arrayfun(@(z) z.bg.range(i,scanline),snap);
  plot(frame+i/(nbg+1),range,col(i));
  leg{end+1}=sprintf('Bg%d',i);
end
legend(leg);
xlabel('Frame');
ylabel('Range');

subplot(312);
leg={};
for i=1:nbg
  sigma=arrayfun(@(z) z.bg.sigma(i,scanline),snap);
  plot(frame+i/(nbg+1),sigma,col(i));
  hold on;
  leg{end+1}=sprintf('Bg%d',i);
end
legend(leg);
xlabel('Frame');
ylabel('Sigma');

subplot(313);
freq=[];
for i=1:nbg
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
