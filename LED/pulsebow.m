%s1=arduino_ip(1);
pperiod=0.3;  % Period of pulsing (in seconds)
cperiod=20;   % Period of color rotation
pspatial=250;  % Spatial period of pulsing
cspatial=20000;  % Spatial period of color
maxlev=1;
minlev=0.2;
dt=0.05;
% Amplitude overall
col=[1 1 1];

p0=(0:numled()-1)*2*pi/pspatial;
pshift=[p0;p0;p0]';
c0=(0:numled()-1)*2*pi/cspatial;
cshift=[c0;c0+2*pi/3;c0+4*pi/3]';
t=0;
while true
  pphase=mod(t*2*pi/pperiod+pshift,2*pi);
  cphase=mod(t*2*pi/cperiod+cshift,2*pi);
  amp=minlev+(maxlev-minlev)*(sin(pphase)+1)/2;
  col=(sin(cphase)+1)/2;
  lev=((amp.*col).^2*.97+.03) * 127;   % Response is nonlinear (approx squared)
  fprintf('t=%.1f, phase=%.0f, amp(1,:)=%.2f %.2f %.2f,lev=%.0f %.0f %.0f\n',t(1,1),phase(1,1)*180/pi,amp(1,:),lev(1,:));
  setallleds(s1,lev,1);
  show(s1,dt);   % Show for dt seconds
  sync(s1);
  t=t+dt;
end
