% Rectify an image
function ir=rectify(im,d,scale)
if nargin<3
  scale=1;
end
offset=round([size(im,1),size(im,2)]*(scale-1)/2);
imtmp=zeros(size(im,1)+2*offset(1),size(im,2)+2*offset(2),size(im,3));
offset=floor((size(imtmp)-size(im))/2);
imtmp(offset(1)+1:offset(1)+size(im,1),offset(2)+1:offset(2)+size(im,2),:)=im2double(im);
cc(1)=d.cc(1)+offset(2);
cc(2)=d.cc(2)+offset(1);
KK = [d.fc(1) d.alpha_c*d.fc(1) cc(1);0 d.fc(2) cc(2); 0 0 1];

if size(imtmp,3)==1
  ir=rect(imtmp,eye(3),d.fc,cc,d.kc,d.alpha_c,KK,1);
else
  ir=rect(imtmp(:,:,1),eye(3),d.fc,cc,d.kc,d.alpha_c,KK,1);
  ir(:,:,2)=rect(imtmp(:,:,2),eye(3),d.fc,cc,d.kc,d.alpha_c,KK,1);
  ir(:,:,3)=rect(imtmp(:,:,3),eye(3),d.fc,cc,d.kc,d.alpha_c,KK,1);
end

