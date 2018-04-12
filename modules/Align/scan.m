nproj=4;
navg=4;
nlevels=6;
night=true;

if night
  exptime=80;
  analoggain=4;
else
  exptime=20;
  analoggain=1;
end
  
p=struct('camera',struct('id',2,'physid',2));
setupcameras(p,'exptime',exptime,'analoggain',analoggain);
imsize=512;
pos=1:imsize;
fnames={};
dirs={'v','h'};
negpos={'','n'};
for level=1:9
  im=false(imsize);
  stripsize=imsize/2^level;
  for i=1:imsize
    im(i,:)=mod(floor((i-1)/stripsize),2)>0;
  end
  for dir=1:2
    for np=1:2
      name=sprintf('align%d%s%s.png',level,dirs{dir},negpos{np});
      imwrite(im,name,'BitDepth',1);
      im=~im;
      fnames{level}{dir}{np}=name;
    end
    im=im';
  end
end

fprintf('Waiting for camera to stabilize...');
pause(10);
fprintf('done\n');

allim={};
for k=1:nproj
  oscmsgout('MM',sprintf('/surfaces/Mask%d/visible',k),{1}); % Madmapper needs on before off
  oscmsgout('MM',sprintf('/surfaces/Mask%d/visible',k),{0});
end
for proj=1:nproj
  oscmsgout('MM',sprintf('/surfaces/Quad%d/visible',proj),{1});
  for k=1:nproj
    if k~=proj
      oscmsgout('MM',sprintf('/surfaces/Quad%d/visible',k),{1});	% Madmapper needs on before off
      oscmsgout('MM',sprintf('/surfaces/Quad%d/visible',k),{0});
    end
  end
  oscmsgout('MM',sprintf('/surfaces/Quad%d/select',proj),{});
  for level=1:nlevels
    for dir=1:2
      for rep=1:navg
        for np=1:2
          name=fnames{level}{dir}{np};
          oscmsgout('MM',sprintf('/medias/%s/select',name),{});
          pause(0.5);
          tmp=arecont(p.camera.id,1);
          allim{proj,level,dir,np,rep}=rgb2gray(tmp.im);
          fprintf('P%d.L%d.%s.%d%s: range=[%.0f,%.0f], mean=%.2f, median=%.2f\n', proj, level, dirs{dir}, rep, negpos{np}, min(tmp.im(:)), max(tmp.im(:)), mean(double(tmp.im(:))),median(tmp.im(:)));
        end
      end
    end
  end
end
analyze
%save('allim.mat','allim','-v7.3');
