% Realtime run script
%plots={'hypo'};
plots={};
countdown=0;

% Setup data structure
global recvis
if exist('recvis','var') && isfield(recvis,'vis') && ~isempty(recvis.vis) && ~isfield(recvis,'note') && length(recvis.vis)>=5
    z=input(sprintf('Are you sure you want to overwrite existing recvis with %d samples? [Y/N]: ',length(recvis.vis)),'s');
    if isempty(z) || upper(z)~='Y'
        return;
    end
end
recvis=struct('p',p,'vis',[],'tgtestimate',[],'possible',{{}},'randseed',rand());

% Save in well known location
save('/tmp/pulsefield_setup.mat','-struct','recvis');

%zz=input('Start frontend in another instance, press return when ready:','s');

s1=arduino_ip(1);
% Turn off LEDS
setled(s1,-1,[0,0,0],1); show(s1); sync(s1);

if countdown>0
  % Sleep a bit
  fprintf('Starting in ');
  for i=countdown:-1:1
    pause(1);
    fprintf('%d...',i);
  end
end

% Turn on LEDs
setled(s1,[0,numled()-1],127*p.colors{1},1); show(s1); sync(s1);

% Wait for them to stabilize
pause(1);

fprintf('Ready\n');

oscdests={'SG','LD','TO'};
% Setup destination for outgoing OSC messages
for i=1:length(oscdests)
  [host,port]=getsubsysaddr(oscdests{i});
  if ~isempty(host)
    oscadddest(['osc.udp://',host,':',num2str(port)])
  end
end

% Start acquisition
mainloop;

% Turn off LEDs
setled(s1,-1,[0,0,0],1);show(s1);
