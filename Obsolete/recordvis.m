% recordvis - record LED states 
function recvis=recordvis(p,nsamps)
s1=arduino_ip();
% Turn on all LED's
fprintf('Turning on LEDs\n');
setled(s1,[0,length(p.led)-1],127*p.colors{1},1);
show(s1);
sync(s1);
% even sending second sync does not ensure that the strip has been set
% pause for 300ms (200ms sometimes wasn't long enough)
pause(0.3);

recvis=struct('p',p,'vis',[]);

for i=1:nsamps
  vis=getvisible(p);
  fprintf('Got recording %d at %s\n',length(recvis.vis)+1,datestr(vis.when));
  recvis.vis=[recvis.vis,vis];
end
fprintf('Captured %d samples in %.2f seconds/sample\n', nsamps,...
        (recvis.vis(end).when-recvis.vis(1).when)/(nsamps-1)*24*3600);

% Turn off LEDs
fprintf('Turning off LEDs\n');
setled(s1,-1,[0,0,0]);
show(s1);

saverecvis