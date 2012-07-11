s1=arduino_ip;
sync(s1);
nset=0;
nled=numled();
level=1;
red=setled(s1,0,level*[1,0,0],0);
green=setled(s1,0,level*[0,1,0],0);
yellow=setled(s1,0,level*[1,1,0],0);
off=setled(s1,0,level*[0,0,0],0);

Fs=44100;
file='/tmp/piano.wav';
time=60;
y=wavread(file,round(time*Fs));

% at t = ta into step, env = 0.63 Vp
ta = 0.005/2.197;  % attack time constant, seconds
tr = 1.7/2.197;  % release time constant, seconds

ga = exp( -1.0 / (Fs * ta) );
gr = exp( -1.0 / (Fs * tr) );

fprintf( '\nta = %f s\ntr = %f s\nga = %f\ngr = %f\n\n', ta, tr, ga, gr );

len = length( y );
env = zeros( len+1, 1 );
for i = 2:(len+1),
  % full-wave rectification
  in = abs( y(i-1) );

  % envelope
  if( in > env(i-1) ),
    % attack
    env(i) = (1-ga) * in + ga * env(i-1);
  else,
    % release
    env(i) = (1-gr) * in + gr * env(i-1);
  end;
end;

vuv=20*log10(env/max(env));
clf;
plot((1:length(vuv))/Fs,vuv);
c=axis;
axis([c(1),c(2),-60,0]);
range=40;
p=audioplayer(y,Fs);
play(p);
tic;
map=[-60,1
     -50,5
     -40,round(nled/8)
     -30,round(nled/4)
     -20,round(nled/2)
     0,nled];
k=0;
while strcmp(p.running,'on')
  fprintf('%d\n',p.CurrentSample);
  lev=interp1(map(:,1),map(:,2),vuv(p.CurrentSample));
  setled(s1,[0,lev-1],level*[0,1,0],1);
  setled(s1,[lev,nled-1],[0,0,0],1);
  show(s1,0.1);
  nset=nset+1;
  sync(s1);
end
setled(s1,-1,[0,0,0],1);
show(s1);
elapsed=toc;
fprintf('Changed LEDs %d times in %.2f seconds = %.2f changes/second\n', nset, elapsed, nset/elapsed);
