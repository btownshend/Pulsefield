% Turn off autoexposure on all cameras so they don't change
function lockexposure(p,lockon)
if nargin<2
  lockon=1;
end
if lockon
  fprintf('Turning off autoexposure on all cameras\n');
else
  fprintf('Turning on autoexposure on all cameras\n');
end
fprintf('Analog gains: ');
for i=1:length(p.camera)
  id=p.camera(i).id;
  if lockon
    arecont_set(id,'autoexp','off');
  else
    arecont_set(id,'autoexp','on');
  end
  fprintf('%d/%d ',arecont_get(id,'reg_3_209'),64*arecont_get(id,'analoggain'));
end
fprintf('\n');

