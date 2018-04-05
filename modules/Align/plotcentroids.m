function plotcentroids(pmap,nofig)
  if nargin==1 || ~nofig
    setfig(['centroids - ',pmap.lbl]);clf;
  end
  im=im2double(pmap.maxim);
  rescale=1.0/prctile(im(:),99);
  imshow((im*rescale).^0.4);
  hold on;
  plot(pmap.centroid(:,:,1),pmap.centroid(:,:,2),'-');
  plot(shiftdim(pmap.centroid(:,:,1),1),shiftdim(pmap.centroid(:,:,2),1),'-');
  title('Centroids');
  xlabel('X');
  ylabel('Y');
end
