% Run what's needed after the system moves or otherwise needs recalibration
input('Make sure Pulsefield is clear, hit return when ready: ','s');
if isfield(p,'pixcalib')
  p.oldpixcalib=p.pixcalib;
  p.pixcalib=pixcalibrate(p);
end
[~,p]=getvisible(p,'init');
for c=1:length(p.camera)
  fprintf('Camera %d can see %d LEDs: %s\n', c, sum(p.camera(c).viscache.inuse),shortlist(find(p.camera(c).viscache.inuse)));
end