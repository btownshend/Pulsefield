% Test PLL sync to MAX transport
usemax=0;
useal=1;

pll=PLL();
starttime=now;
dur=30;
all=[];

fprintf('Running PLL test for %d seconds\n', dur);
all=[];
flush=true;  % Flush messages first time through
while (now-starttime)*24*3600 < dur
  while true
    if usemax
      msg=oscmsgin('MPO',0.01);
    else
      msg=oscmsgin('MPA',0.01);
    end
    
    when=now;
    if isempty(msg)
      break;
    end
    if flush
      continue;
    end
    pos=0;
    if usemax && strcmp(msg.path,'/max/transport')
      % Received bars,beats,ticks,res,tempo,time sig1, time sig2
      pos=msg.data{1}*msg.data{6}+msg.data{2}+msg.data{3}/480;
      pll.settempo(msg.data{5});
    end
    if useal
      if strcmp(msg.path,'/live/beat')
        % Received bars,beats,ticks,res,tempo,time sig1, time sig2
        pos=msg.data{1};
      end
      if strcmp(msg.path,'/live/tempo')
        pll.settempo(msg.data{1});
      end
    end
    if pos~=0
      pll.setref(pos,when);
      beat=pll.getbeat(when);
      curtime=(when-starttime)*24*3600;
      all=[all,struct('curtime',curtime,'pos',pos,'clkbeat',pll.clkbeat,'bpm',pll.bpm,'freq',pll.freq,'pdlpf',pll.pdlpf)];
    end
  end
  flush=false;
end
if length(all)<=1
  error('No messages received');
end

fprintf('Got %d transport messages in %d seconds\n', length(all), dur);
setfig('pll'); 
clf;
subplot(311);
plot([all.curtime],[all.pos],'.g');
hold on
plot([all.curtime],[all.clkbeat],'r');
legend('OSC','Clk');
xlabel('Time (s)');
ylabel('Transport position');
subplot(312);
plot([all.curtime],[all.pdlpf],'r');
hold on
plot([all.curtime],[all.pos]-[all.clkbeat],'.g');
legend('PDLPF','PD');
xlabel('Time (s)');
ylabel('Phase Difference');
subplot(313);
plot([all.curtime],[all.freq]*60,'r');
hold on;
plot([all.curtime],[all.bpm],'.g');
legend('Tracked','Nominal');
xlabel('Time (s)');
ylabel('Freq (BPM)');
