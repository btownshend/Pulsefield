pmap={};
for i=1:size(allim,1)
  pmap{i}=buildpmap(sprintf('P%d',i),squeeze(allim(i,:,:,:,:)));
end
save('pmap.mat','pmap');
