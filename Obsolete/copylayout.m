% Copy layout to p structure
function p=copylayout(p,layout)
for i=1:length(p.camera)
  p.camera(i).pos=layout.cpos(i,:);
  p.camera(i).dir=layout.cdir(i,:);
end
for i=1:length(p.led)
  p.led(i).pos=layout.lpos(i,:);
  p.led(i).dir=layout.ldir(i,:);
end
