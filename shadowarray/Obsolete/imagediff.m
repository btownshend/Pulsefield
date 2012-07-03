numimages=10;
if ~exist('im')
  im=[];
  for i=1:numimages
    im(i,:,:,:)=dcsget();
    pause(1);
  end
  im=im(:,100:end,:,:);
end

s=squeeze(std(im(:,:,:,:),0,1));
s=sum(s,3);
figure(1);
clf;
imshow(s/(max(s(:))-min(s(:)))+min(s(:)));
% Find the pixels whos stdev over time was the highest
[ms,os]=sort(s(:),'descend');
keepcnt=10;
[ii,jj]=ind2sub(size(s),os(1:keepcnt));
changes=struct();
for i=1:length(ii)
  changes(i).i=ii(i);
  changes(i).j=jj(i);
  changes(i).v=squeeze(im(:,ii(i),jj(i),1));
  changes(i).s=s(ii(i),jj(i));
end
figure(2);
clf;
col='rgb';
for c=1:3
  plot(im(:,i(1),j(1),c),col(c));
  hold on;
end
title(sprintf('(%d,%d)',i,j));