% Realtime run script

% Setup data structure
recvis=struct('p',p,'layout',layout,'rays',rays,'vis',[],'tgtestimate',[],'possible',{{}},'randseed',rand());

% Save in well known location
save('/tmp/pulsefield_setup.mat','-struct','recvis');

zz=input('Start frontend in another instance, press return when ready:');

% Turn on LEDs
s1=arduino_ip(1);
setled(s1,[0,numled()-1],p.colors{1},1); show(s1); sync(s1);

% Start acquisition

%recvis=mainloop(recvis,[],{'hypo'});
recvis=mainloop(recvis,[],{});

% Turn off LEDs
setled(s1,-1,[0,0,0],1);show(s1);
