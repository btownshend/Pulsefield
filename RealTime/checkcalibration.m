% Check if any cameras/LEDs have moved
function checkcalibration(p,vis,leds)
if nargin<3
  nl=4;
  leds=round(1:(numled()-1)/(nl-1):numled());
else
  nl=length(leds);
end

wind=5;
if nargin<2 || isempty(vis)
  % Use lower level to avoid clipping
  vis=getvisible(p,'onval',20*[1 1 1],'stats');
end

setfig('checkcalibration');
clf;
nc=length(p.camera);
if isfield(vis,'tgt')
  nstack=3;
else
  nstack=1;
end

for c=1:nc
  pixcalib=p.camera(c).pixcalib;
  fvalid=find([pixcalib.valid]);
  fprintf('Camera %d, LEDs = %s\n',shortlist(fvalid));
  first=1;
  for i=1:nl
    l=leds(i);
    if ~isempty(pixcalib(l).pixelList)
      subplot(nc,nl*nstack,(c-1)*nl*nstack+(i-1)*nstack+1);
      plist=pixcalib(l).pixelList;
      roi=p.camera(c).roi;
      plist(:,1)=plist(:,1)-roi(1)+1;
      plist(:,2)=plist(:,2)-roi(3)+1;
      for j=1:2
        rng{j}=max(1,min(plist(:,j))-wind):min(size(vis.im{c},3-j),max(plist(:,j))+wind);
      end
      imled=im2double(vis.im{c}(rng{2},rng{1},:));
      imshow(imled);
      %    imshow(imled/max(imled(:)));
      hold on;
      for k=1:size(plist,1)
        plot(-(rng{1}(1)-1)-0.5+plist(k,1)+[0 1 1 ],-(rng{2}(1)-1)-0.5+plist(k,2)+[1 1 0 ],'r');
      end
      if first
        ylabel(sprintf('Camera %d',c));
        first=0;
      end
      if pixcalib(l).valid
        title(sprintf('L%d\n', l));
      else
        title(sprintf('*L%d\n', l));
      end
      if nstack>1
        subplot(nc,nl*nstack,(c-1)*nl*nstack+(i-1)*nstack+2);
        imshow(vis.tgt{c,l});
        title('Tgt');
        xlabel(sprintf('p=%.2f',vis.corr(c,l)));
        subplot(nc,nl*nstack,(c-1)*nl*nstack+(i-1)*nstack+3);
        imshow(p.camera(c).viscache.ref{l});
        title('Ref');
      end
    end
  end
end