function plotbgranges(csnap,scan)
range=arrayfun(@(z) z.vis.range(scan), csnap);
bglike=arrayfun(@(z) z.tracker.bglike(scan), csnap);
minfreq=.01;
for i=1:5
  bg{i}=arrayfun(@(z) z.bg.range(i,scan), csnap);
  sigma{i}=arrayfun(@(z) z.bg.sigma(i,scan), csnap);
  freq{i}=arrayfun(@(z) z.bg.freq(i,scan), csnap);
end
frame=arrayfun(@(z) z.vis.frame, csnap);
setfig('bgranges');clf;
subplot(211);
h=plot(frame,range,'k');
hold on;
leg={'Scan'};
cols='rgbcmy';
for i=1:5
  if any(freq{i}>=minfreq)
    tmp=bg{i};
    %    tmp(~sel)=nan;
    yyaxis left;
    h(end+1)=plot(frame,tmp,cols(i));
    plot(frame,tmp-sigma{i},[cols(i),':']);
    plot(frame,tmp+sigma{i},[cols(i),':']);
    leg{end+1}=sprintf('BG%d',i);
    yyaxis right;
    plot(frame,freq{i},['--',cols(i)]);
    hold on;
  end
end
legend(h,leg);
xlabel('Frame');
yyaxis left
ylabel('Range');
yyaxis right
ylabel('Freq');
title(sprintf('BG Scan %d',scan));
subplot(212);
yyaxis left
semilogy(frame,exp(bglike)/1000);
ylabel('BG Prob');
c=axis;
c(3)=exp(0)/1000;
c(4)=1.05;
axis(c);
yyaxis right
plot(frame,bglike);
ylabel('BG Like');
c=axis;
c(3)=0;
c(4)=log(1000*1.05);
axis(c);

xlabel('Frame');
