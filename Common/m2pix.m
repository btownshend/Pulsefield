% m2pix - convert meters location to pixels
function pix=m2pix(imap,m)
if size(m,2)==1
  % Unidimensional - assume its a length
  if length(imap.scale)>1
    if imap.scale(1)~=imap.scale(2)
      error('Can''t scale length with unequal scale factors');
    end
  end
  pix=m*imap.scale(1);
else
  pix=m;
  for i=1:size(m,1)
    pix(i,:)=(pix(i,:)-imap.origin).*imap.scale;
  end
end
