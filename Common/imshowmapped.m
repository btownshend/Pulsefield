%imshowmapped - imshow with coords set correctly
function imshowmapped(imap,im);
tl=pix2m(imap,[1,1]);
br=pix2m(imap,[size(im,1),size(im,2)]);
if ndims(im)==3
  im=permute(im,[2 1 3]);
else
  im=im';
end
imshow(im,'XData',[tl(1),br(1)],'YData',[tl(2),br(2)]);
axis on;
axis xy
