% OLD stuff..

for i=1:length(im{1})
  setfig(sprintf('C%d',i));
  clf;
  z=[];
  for k=1:length(im)
    zk=sum(im2double(im{k}{i}),3);
    fprintf('i=%d,k=%d,mean=%.2f\n',i,k,mean(zk(:)));
    z(k,1:size(zk,1),1:size(zk,2))=zk; %/mean(zk(:));
  end
  zz=squeeze(std(z,0,1));
  %  z=z/(mean(z(:))/4);
  fprintf('max(zz)=%.3f, mean=%.3f, median=%.3f\n', max(zz(:)),mean(zz(:)),median(zz(:)));
  imshow(zz*2);
  hold on;
  ppos=[nan,nan];
  for j=1:length(p.camera(i).pixcalib)
    pc=p.camera(i).pixcalib(j);
    pos=pc.pos;  %-[p.camera(i).roi(1),p.camera(i).roi(3)];
    if isfinite(ppos(1)) & isfinite(pos(1)) & norm(ppos-pos)<10
      plot([ppos(1),pos(1)],[ppos(2),pos(2)],'-r');
    end
    ppos=pos;
  end
end
