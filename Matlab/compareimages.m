ic=im2double(imread('/tmp/ofcapt-den.tif'));
ij=im2double(imread('/tmp/canvas.tif'));
setfig('Images');clf;
subplot(221);
imshow(ic(:,:,1:3));
title('C++');
subplot(222);
imshow(ic(:,:,4));
title('C++ alpha');
subplot(223);
imshow(ij);
title('Java');
subplot(224);
imshow((ij-ic(:,:,1:3)+0.5))
title('Java-C++');


sel=1:100:size(compare,1);
setfig('Alpha Times');clf;
lbls={'Red','Green','Blue','Alpha'};
compare=[reshape(ic(:,:,1:4),[],4),reshape(ij(:,:,1:3),[],3)];
cols='rgb';
subplot(121);
for c=1:3
  plot(compare(sel,c).*compare(sel,4),compare(sel,c+4),['.',cols(c)]);
  hold on;
end
%c=axis;c(1)=0;c(3)=0; axis(c);
axis([0,1,0,1]);
axis equal
legend('R','G','B');
xlabel('C++ color*alpha');
ylabel('Java color');

subplot(122);
for c=1:3
  plot(compare(sel,c),compare(sel,c+4),['.',cols(c)]);
  hold on;
end
%c=axis;c(1)=0;c(3)=0; axis(c);
axis([0,1,0,1]);
axis equal
legend('R','G','B');
xlabel('C++ color');
ylabel('Java color');

fprintf('Alpha = [%d,%d], mean %.1f\n', min(compare(:,4)), max(compare(:,4)), mean(compare(:,4)));
