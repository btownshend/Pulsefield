function monitor
dev=audiodevinfo();
for i=1:length(dev.input)
  d=dev.input(i);
  fprintf('Input device %d: %s\n', d.ID, d.Name);
end
ar=audiorecorder(8000,16,1,0)
global env lastsamp s1
s1=arduino_ip;
env=0;
lastsamp=0;
clf;
ar.TimerFcn=@timer;
ar.TimerPeriod=0.05;
record(ar);
while true
  if 0 && ar.CurrentSample>80000
    stop(ar);
    record(ar);
    fprintf('Reset\n');
  end
  pause(1);
end
stop(ar);

  
function timer(ar,ev)
global env lastsamp s1
nled=160;
cursamp=ar.CurrentSample;
if cursamp<2
  return
end
x=getaudiodata(ar,'int16');
% at t = ta into step, env = 0.63 Vp
ta = 0.005/2.197;  % attack time constant, seconds
tr = 0.7/2.197;  % release time constant, seconds
Fs = 8000;
ga = exp( -1.0 / (Fs * ta) );
gr = exp( -1.0 / (Fs * tr) );

x=double(abs(x(lastsamp+1:end)))/32768.0;
%x=double(abs(x))/32768;
lastsamp=lastsamp+length(x);
len = length( x );
for i = 1:length(x)
  % envelope
  if( x(i) > env)
    % attack
    env = double((1-ga) * x(i)) + ga * env;
  else,
    % release
    env = double((1-gr) * x(i)) + gr * env;
  end;
end;

vulevel=20*log10(env);
fprintf('Current sample=%d/%d, length(x)=%d, max(x)=%d, VU=%.1f\n', cursamp, ar.TotalSamples,length(x), max(x), vulevel);
plot(lastsamp/Fs,vulevel,'.');
%plot(lastsamp/Fs, 20*log10(max(x)),'r.');
hold on;
bk=[35,75,115];
map=[-60,1
     -50,5
     -40,10
     -30,40
     -20,80
     0,bk(1)+bk(3)-bk(2)];
lev=round(interp1(map(:,1),map(:,2),vulevel));
level=1;
%setwave(s1,160*0+[0,lev-1],level*[1,0,0;0,0,1]);
%setwave(s1,160*1+[0,lev-1],level*[1,0,0;0,1,0]);
%setwave(s1,160*2+[0,lev-1],level*[1,0,0;0,1,1]);
%setwave(s1,160*3+[0,lev-1],level*[0,1,0;0,0,1]);
cols=level*[0 1 0
            0 0 1
            1 0 1
            1 0 0];
cmd=[];
if lev>bk(1)
  lev=lev+bk(2)-bk(1);
end
for i=1:4
  cmd=[cmd,setled(s1,160*(i-1)+[0,min(bk(1),lev-1)],cols(i,:))];
  if lev>bk(1)
    cmd=[cmd,setled(s1,160*(i-1)+[bk(2),min(bk(3),lev-1)],cols(i,:))];
  end
  cmd=[cmd,setled(s1,160*(i-1)+[lev,nled-1],[0,0,0])];
end
cmd=[cmd,'G'];
awrite(s1,cmd);
sync(s1,3);
