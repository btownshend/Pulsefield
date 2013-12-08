img='20131115-c2/c2-33.jpg';
im=imread(img);
imshow(im);
I=im2double(im);
I = 0.299 * I(:,:,1) + 0.5870 * I(:,:,2) + 0.114 * I(:,:,3);
c=findcorners(I,1);
