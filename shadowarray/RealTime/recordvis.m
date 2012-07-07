% recordvis - record LED states 
function recvis=recordvis(p,layout,rays,nsamps)
s1=arduino_ip();
% Turn on all LED's
fprintf('Turning on LEDs\n');
setled(s1,[0,length(p.led)-1],[50,50,50],1);
show(s1);
sync(s1);
% even sending second sync does not ensure that the strip has been set
% pause for 300ms (200ms sometimes wasn't long enough)
pause(0.3);

recvis=struct('p',p,'layout',layout,'rays',rays,'vis',[]);

im=cell(1,nsamps);
for i=1:nsamps
  vis=getvisible(p,0);
  fprintf('Got recording %d at %s\n',length(recvis.vis)+1,datestr(vis.when));
  recvis.vis=[recvis.vis,rmfield(vis,'im')];
  % Store separately so it doesnt cause save failure
  recvis.im{i}=vis.im;
end
fprintf('Captured %d samples in %.2f seconds/sample\n', nsamps,...
        (recvis.vis(end).when-recvis.vis(1).when)/(nsamps-1)*24*3600);

% Turn off LEDs
fprintf('Turning off LEDs\n');
setled(s1,-1,[0,0,0]);
show(s1);

recvis.note=input('Enter comment for save file (or return to skip save): ','s');
if ~isempty(recvis.note)
  recname=sprintf('Recordings/%s.mat',datestr(vis.when,30));
  fprintf('Saving recording in %s...', recname);
  save(recname,'-struct','recvis');
  fprintf('done\n');
end
