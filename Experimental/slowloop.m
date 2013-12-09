% Run updating analyze output, saving vis
while true
  vis=getvisible(p,'stats',true,'usefrontend',true);
  analyze(p,vis.v,3);
end
