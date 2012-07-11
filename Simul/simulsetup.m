function sim=simulsetup
% Target parameters
sim=struct();
sim.speed=1.3;   % m/s
sim.meanstoptime=2;  % Mean stopping time
sim.meanwalkingtime=2;	% Mean walking time
sim.stddirchange=30;	% Change in walking direction in degrees/second, while walking

% Sampling speed
sim.dt=0.1;	% seconds