function plotblocked(vis)
idata=zeros(length(vis(1).vorig(1,:)),length(vis),3,'uint8');
colors=[255,0,0
        0,255,0
        127,127,0
        0,127,127
        127,0,127
        0,0,255];
for i=1:6
  for k=1:length(vis)
    idata(:,k,:)=colors(vis(k).vorig(i,:)+1,:);
  end
  subplot(3,2,i);
  imshow(idata);
  title(sprintf('Camera %d',i));
  xlabel('Frame');
end
