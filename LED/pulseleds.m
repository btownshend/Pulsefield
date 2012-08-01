s1=arduino_ip;
sync(s1);
period=2.0;  % Period of pulsing (in seconds)
maxlev=0.6;
minlev=0.4;
minradius=0.5;   % Radius where lights are off
dt=0.033;
col=[1 1 1];

% Amplitude overall
t=0;
while true
  phase=t*2*pi/period;
  amp=minlev+(maxlev-minlev)*(sin(phase)+1)/2;
  lev=(amp.^2*.97+0.03) * 127;   % Response is nonlinear (approx squared)
  fprintf('t=%.1f, amp=%.2f,lev=%.0f\n',t,amp,lev);

  % Send new values
  s1=arduino_ip(0);
  setled(s1,-1,round(lev*col),1); 
  show(s1,dt);   % Show for at least 0.1s
  tic;sync(s1);toc
  t=t+dt;
end