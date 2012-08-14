s1=arduino_ip;
sync(s1);
period=2.0;  % Period of pulsing (in seconds)
maxlev=0.6;
minlev=0.2;
minradius=0.5;   % Radius where lights are off
dt=0.032167;
col=[1 1 1];

% Amplitude overall
t=0;
cnt=0;
while true
  phase=t*2*pi/period;
  amp=minlev+(maxlev-minlev)*(sin(phase)+1)/2;
  lev=(amp.^2*.97+.03) * 127;   % Response is nonlinear (approx squared)
  fprintf('t=%.1f, amp=%.2f,lev=%.0f\n',t,amp,lev);

  % Send new values
  s1=arduino_ip(0);
  col=col+rand(size(col))/5;
  col=col/max(col);
  setled(s1,-1,round(lev*col),1); 
  show(s1,dt);   % Show for at least 0.1s
  if mod(cnt,20)==0
    tic;sync(s1);toc
    fprintf('Expected = %.2f seconds\n', 20*dt);
  end
  t=t+dt;
  cnt=cnt+1;
end