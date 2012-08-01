% Test C++ frontend 
% 
% Initialize frontend
rcvr(p,'init');

% Turn on LEDs
s1=arduino_ip();
setled(s1,-1,[127,0,0],1); show(s1);
pause(1);

% Retrieve snapshot
vis=rcvr(p,'stats');
assert(~isempty(vis));
plotvisible(p,vis,'PV: From Frontend');

% Compute statistics on same images, using MATLAB version 
matvis=getvisible(p,'im',vis.im,'stats',true);
plotvisible(p,matvis,'PV: MATLAB getvisible');

% Check for maximum difference
cdiff=abs(vis.corr-matvis.corr);
[maxdiff,ind]=max(cdiff(:));
[camera,led]=ind2sub(size(cdiff),ind);
fprintf('Corr(%d,%d) = %.3f (MATLAB), %.3f (Frontend) Difference=%.3f\n', camera, led, matvis.corr(camera,led), vis.corr(camera,led), cdiff(camera,led));

% Check a particular tgt vs. refim
checkcalibration(p,vis,led,'CC: From Frontend');
checkcalibration(p,matvis,led,'CC: MATLAB getvisible');
