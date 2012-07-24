% Realtime run script
%plots={'hypo'};
plots={};

% Setup data structure
global recvis
if exist('recvis') && isfield(recvis,'vis') && ~isempty(recvis.vis) && ~isfield(recvis,'note')
    z=input(sprintf('Are you sure you want to overwrite existing recvis with %d samples? [Y/N]: ',length(recvis.vis)),'s');
    if isempty(z) || upper(z)~='Y'
        return;
    end
end
recvis=struct('p',p,'layout',layout,'rays',rays,'vis',[],'tgtestimate',[],'possible',{{}},'randseed',rand());

% Save in well known location
save('/tmp/pulsefield_setup.mat','-struct','recvis');

%zz=input('Start frontend in another instance, press return when ready:','s');

s1=arduino_ip(1);
% Turn off LEDS
setled(s1,-1,[0,0,0],1); show(s1); sync(s1);

% Sleep a bit
fprintf('Starting in ');
for i=10:-1:1
  fprintf('%d...',i);
  pause(1);
end
fprintf('Ready\n');

% Turn on LEDs
setled(s1,[0,numled()-1],p.colors{1},1); show(s1); sync(s1);

% Start acquisition
mainloop;

% Turn off LEDs
setled(s1,-1,[0,0,0],1);show(s1);
