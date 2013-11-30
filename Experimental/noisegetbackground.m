% Acquire noise background
function bg=noisegetbackground(p,nframes)
for i=1:nframes
  vis=getvisible(p,'stats',true);
  bg{i,:}=vis.im(:);
end