% Analyze image sequence in allim{level,dir,np,rep}
function pmap=buildpmap(lbl,allim)
  maxlevels=6;
  sz=size(allim{1});
  mapxpos=zeros(sz(1),sz(2),'int32');
  mapxneg=zeros(sz(1),sz(2),'int32');
  mapypos=zeros(sz(1),sz(2),'int32');
  mapyneg=zeros(sz(1),sz(2),'int32');
  nlevels=min(maxlevels,size(allim,1));
  setfig(['buildpmap - ',lbl]);clf;
  maxim=allim{1};
  for level=1:nlevels
    for dir=1:2
      % Reduce to a map that is <0 for neg, >0 for pos, 0 for unused
      im=reduceimages(squeeze(allim(level,dir,:,:)));
      maxim=max(maxim,allim{level,dir,1,1});
      maxim=max(maxim,allim{level,dir,2,1});
      pos=im>0;
      neg=im<0;
      res=im;
      res(:,:,1)=pos==neg;
      res(:,:,2)=pos*255;
      res(:,:,3)=neg*255;
      subplot(ceil(nlevels/2),4,(level-1)*2+dir);
      imshow(res);
      pause(0.1);
      bit=floor(nlevels-level);
      if dir==1
        mapxpos(find(pos))=mapxpos(find(pos))+2^bit;
        mapxneg(find(neg))=mapxneg(find(neg))+2^bit;
      else
        mapypos(find(pos))=mapypos(find(pos))+2^bit;
        mapyneg(find(neg))=mapyneg(find(neg))+2^bit;
      end
    end
  end

  maxval=2^nlevels-1;
  setfig(['pos/neg - ',lbl]);clf;
  subplot(2,2,1);
  imshow(mapxpos,[0,maxval]);
  title('xpos');
  subplot(2,2,2);
  imshow(mapxneg,[0,maxval]);
  title('xneg');
  subplot(2,2,3);
  imshow(mapypos,[0,maxval]);
  title('ypos');
  subplot(2,2,4);
  imshow(mapyneg,[0,maxval]);
  title('yneg');

  finalx=mapxpos+1;
  validx=mapxpos+mapxneg==maxval;
  finalx(~validx)=0;
  finaly=mapypos+1;
  validy=mapypos+mapyneg==maxval;
  finaly(~validy)=0;

  setfig(['finalx - ',lbl]);clf;
  imshow(finalx,[0,maxval+1]);
  title('X');

  setfig(['finaly - ',lbl]);clf;
  imshow(finaly,[0,maxval+1]);
  title('Y');

  % Clean up
  minpixfrac=0.01;	% Remove other regions whose size is less than this fraction of the largest region
  for i=1:maxval+1
    % Break into connected regions
    cc=bwconncomp(finalx==i);
    if cc.NumObjects>1
      % Find the biggest
      numPixels = cellfun(@numel,cc.PixelIdxList);
      [biggest,idx] = max(numPixels);
      fprintf('FinalX %d has multiple regions.  Biggest is %d with %d pixels, removing others with ',i, idx, biggest);
      for j=1:cc.NumObjects
        if numPixels(j)<minpixfrac*biggest
          finalx(cc.PixelIdxList{j}) = 0;
          fprintf('%d ',numPixels(j));
        elseif j~=idx
          fprintf('[%d] ',numPixels(j));
        end
      end
      fprintf('\n');
    end
  end

  for i=1:maxval+1
    % Break into connected regions
    cc=bwconncomp(finaly==i);
    if cc.NumObjects>1
      % Find the biggest
      numPixels = cellfun(@numel,cc.PixelIdxList);
      [biggest,idx] = max(numPixels);
      fprintf('FinalY %d has multiple regions.  Biggest is %d with %d pixels, removing others with ',i, idx, biggest);
      for j=1:cc.NumObjects
        if numPixels(j)<minpixfrac*biggest
          finaly(cc.PixelIdxList{j}) = 0;
          fprintf('%d ',numPixels(j));
        elseif j~=idx
          fprintf('[%d] ',numPixels(j));
        end
      end
      fprintf('\n');
    end
  end

  % Build a label matrix that identifies the coordinates of each pixel
  final=(finalx-1)*(maxval+1)+finaly;
  final(finalx==0)=0;
  final(finaly==0)=0;
  stats=regionprops(final,{'Centroid','Extrema','EquivDiameter'});
  diameter=nan(maxval+1,maxval+1);
  centroid=nan(maxval+1,maxval+1,2);
  for i=1:maxval+1
    for j=1:maxval+1
      ind=(i-1)*(maxval+1)+j;
      if ind>length(stats)
        continue;
      end
      diameter(i,j)=stats(ind).EquivDiameter;
      centroid(i,j,:)=stats(ind).Centroid;
    end
  end

  % Interpolate centroids
  upsample=64/size(centroid,1);
  if upsample>1 && false
    fprintf('Upsampling centroid grid by %dx\n', upsample);
    c2=interp2(centroid(:,:,1),upsample);
    c2(:,:,2)=interp2(centroid(:,:,2),upsample);
    centroid=c2;
  end


  % Now build a map that can unwarp a camera image
  % proj2cam(x,y) contains the index in a camera image that maps to (x,y) in projector space 
  % cam2proj(x,y) contains the index in projector space that maps to (x,y) in camera space x=1:1376, y=1:1824
  pmap=struct('centroid',centroid,'cx',((1:size(centroid,1))-0.5)/size(centroid,1),'cy',((1:size(centroid,2))-0.5)/size(centroid,2),'maxim',maxim,'lbl',lbl);

  % Plot centroids
  plotcentroids(pmap);
end
