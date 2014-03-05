function cvpeople(Iref,Inew)
mask=any(abs(im2double(Inew)-im2double(Iref))>0.2,3);
mask = imopen(mask, strel('rectangle', [3,3]));
mask = imdilate(mask, strel('disk', 15));
mask = imerode(mask, strel('disk', 10));
mask = imfill(mask, 'holes');
mask=repmat(mask,[1,1,3]);
I=Inew;
I(~mask)=0;
pd=vision.PeopleDetector('ClassificationModel','UprightPeople_96x48','MaxSize',[200,100]);
[bboxes,scores]=step(pd,I);
shapeInserter = vision.ShapeInserter('BorderColor','Custom','CustomBorderColor',[255 255 0]);
scoreInserter = vision.TextInserter('Text',' %f','Color', [0 80 255],'LocationSource','Input port','FontSize',16);
I = step(shapeInserter, I, int32(bboxes));
I = step(scoreInserter, I, scores, int32(bboxes(:,1:2))); 
setfig('cvpeople');
imshow(I)
title('Detected people and detection scores'); 