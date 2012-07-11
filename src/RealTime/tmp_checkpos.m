% Cross check positions of LED's
pixpos=[];
for i=1:length(sainfo.camera)
  pixpos(:,:,i)=reshape([sainfo.camera(i).pixcalib.pos],2,[]);
end
clf;
plot(pixpos(:,:,1)',pixpos(:,:,3)','.');
