% Check various effects on image
% Register Map:
% 
id=1;
tests={{'brightness',-50,50,0},  % Reg 18 = brightness+100
       {'shortexposures',1,80},  % Reg 32 = shortexposures*19.8;  reg 263= shortexposures+256*60
       {'kneepoint',1,100}, % reg 81 = kneepoint
       {'analoggain',1,10}, % reg 39 = analoggain*64
       {'maxkneegain',2,100},  % reg 95 = min(maxkneegain,30)
       {'maxexptime',0,100},   % reg 53 = maxexptime
       {'maxdigitalgain',32,127}  % reg 60 = maxdigitalgain
      };
arecont_set(id,'autoexp','off');
arecont_set(id,'exposure','off');
arecont_set(id,'lowlight','highspeed');
testvals={};
for i=1:length(tests)
  t=tests{i};
  fprintf('Testing %s\n', t{1});
  posreg=ones(7,256);   % 1 for things that may have an effect
  tv=struct([]);
  r={};
  for k=1:2
    for j=2:length(t)
      arecont_set(id,t{1},t{j});
      v=arecont_get(id,t{1});
      pause(2);
      p=arecont(id);
      imshow(p.im);
      setting=sprintf('Set %s to %d',t{1},v);
      title(setting);
      fprintf('%s: mean level=%f, min=%f, max=%f\n',setting,mean(double(p.im(:))),min(p.im(:)),max(p.im(:)));
      tv(end+1).param=t{1};
      tv(end).value=t{j};
      tv(end).regs=arecont_getregs(id);
      r{k,j-1}=tv(end).regs;
      if j>2
        posreg=posreg & (r{k,j-1}~=r{k,j-2});
      end
      if k>1
        posreg=posreg & (r{k,j-1}==r{k-1,j-1});
      end
    end
  end
  regmap{i}=find(posreg)
  testvals{end+1}=tv;
end
