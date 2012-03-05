waistcircum=1*p.scale;
wcsigma=0.14*p.scale;
meantgtdiam=waistcircum/pi;
sdevtgtdiam=wcsigma/pi;
mintgtdiam=meantgtdiam/1.2-sqrt(3)*sdevtgtdiam;  % 1.2 for eccentricity of shapes
maxtgtdiam=meantgtdiam*1.2+sqrt(3)*sdevtgtdiam;
fprintf('Targets are %.1f-%.1f cm in diameter\n', 100*[mintgtdiam,maxtgtdiam]/p.scale);
ntgt=10;
mincdist=1*p.scale;	% Minimum distance between cameras and targets

% Choose target positions
% Eliminate ones too close to camera or each other
fprintf('Choosing targets\n');
tpos=[];
tgtdiam=[];
for i=1:ntgt;
  tgtdiam(i)=rand(1,1)*(maxtgtdiam-mintgtdiam)+mintgtdiam;
  % Find a legal position
  while true
    tpos(i,:)=(p.sidelength-tgtdiam(i))*rand(1,2)+tgtdiam(i)/2;
    ok=1;
    for j=1:i-1
      if norm(tpos(i,:)-tpos(j,:)) < (tgtdiam(i)+tgtdiam(j))/2
        ok=0;
        break;
      end
    end
    for c=1:size(cpos,1)
      if norm(tpos(i,:)-cpos(c,:)) < mincdist
        ok=0;
        break;
      end
    end
    if ok
      break;
    end
  end
end
fprintf('done\n');
