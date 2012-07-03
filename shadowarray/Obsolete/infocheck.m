% Load an image from iPhone and check info
while true
  [img,info]=dcsget('http://192.168.0.136:12345/image.jpg');
  info.DigitalCamera
  imshow(img);
  pause(1);
end
