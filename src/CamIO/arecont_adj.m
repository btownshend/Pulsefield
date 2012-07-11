% Test adjustment of levels
id=2;
shutter=10;   % msec.
ag=10;
clip=.01;
arecont_set(id,'lowlight','highspeed');
arecont_set(id,'maxdigitalgain',32);
arecont_set(id,'exposure','on');
arecont_set(id,'autoexp','on');
while true
  fprintf('Setting analoggain to %d, speed to %d\n',ag,shutter);
  arecont_set(id,'analoggain',ag);
  arecont_set(id,'shortexposures',shutter);
  pause(5);
  v=arecont(id);
  %imshow(v.im);
  gain=arecont_get(id,'reg_3_209');
  si=sort(v.im(:),'descend');
  mxim=si(round(clip*length(si)));
  medim=si(round(length(si)/2));
  expnum=1000*double(medim)/gain/shutter;
  fprintf('Expnum=%.2f, Gain=%d, avg=%.2f, median=%.2f, %.1fth percentile=%.2f\n',expnum,gain,mean(v.im(:)),medim,(1-clip)*100,mxim);
  if medim<125
    if ag<25
      ag=min(25,ceil(ag*2));
    elseif shutter<80
      shutter=min(80,ceil(shutter*2));
    else
      break;
    end
  elseif medim>250
    if ag>1
      ag=max(1,floor(ag/2));
    elseif shutter>1
      shutter=max(1,floor(shutter/2));
    else
      break;
    end
  else
    break;
  end
end