function p=arecont_bright(id)
arecont_set(id,'autoexp','on');
arecont_set(id,'exposure','on');
arecont_set(id,'brightness',0);
arecont_set(id,'lowlight','quality');
arecont_set(id,'maxdigitalgain',32);
arecont_set(id,'analoggain',1);
pause(5);  % To give time to adjust
p=arecont(id);
