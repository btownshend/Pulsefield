% pix2m - convert to meters location from pixels
function m=pix2m(imap,pix)
m=pix;
for i=1:size(m,1)
  m(i,:)=m(i,:)./imap.scale + imap.origin;
end
