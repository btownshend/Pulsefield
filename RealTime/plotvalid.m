% Plot diagram of invalid camera->led pixcalibs
function plotvalid(p)
setfig('valid');
clf;
for i=1:length(p.camera)
  subplot(3,2,i);
  plotlayout(p.layout,0);
  hold on;
  invalid=find(~[p.camera(i).pixcalib.valid]);
  for j=1:length(invalid)
    l=invalid(j);
    plot([p.layout.cpos(i,1),p.layout.lpos(l,1)],[p.layout.cpos(i,2),p.layout.lpos(l,2)],'r');
  end
  title(sprintf('Invalid leds for camera %d', i));
  pause(0.1);
end