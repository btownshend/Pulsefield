%imshowmapped - imshow with coords set correctly
function imshowmapped(imap,im);
tl=pix2m(imap,[1,1]);
br=pix2m(imap,size(im));
imshow(im','XData',[tl(1),br(1)],'YData',[tl(2),br(2)]);
axis on;
axis xy
