% Acquire noise background
function bg=noisegetbackground(p,nframes)
for i=1:nframes
  vis=getvisible(p,'stats',true);
  for c=1:length(p.camera)
    bg.frames{i,c}=im2double(vis.im{c});
    if i==1
      bgtotal{c}=bg.frames{i,c};
      bgtotal2{c}=bg.frames{i,c}.^2;
    else
      bgtotal{c}=bgtotal{c}+bg.frames{i,c};
      bgtotal2{c}=bgtotal2{c}+bg.frames{i,c}.^2;
    end
  end
end
for c=1:length(bgtotal)
  bg.avg{c}=bgtotal{c}/nframes;
  bg.std{c}=sqrt(bgtotal2{c}/nframes-(bgtotal{c}/nframes).^2);
end
