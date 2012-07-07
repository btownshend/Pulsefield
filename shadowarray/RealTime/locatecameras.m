% locatecameras using blockage near LEDs
function [ncpos,recvis]=locatecameras(recvis)
if nargin<1
  fprintf('Move a black CD case around LED struts near each camera blocking LEDs to far camera and view from near camera\n');
  input('Hit return when ready to start (15s pause will follow)','s');
  pause(15);
  nsamps=150;
  fprintf('Acquiring %d samples\n', nsamps);
  recvis=recordvis(p,layout,rays,nsamps);
end
selcam=zeros(length(recvis.p.camera),length(recvis.vis));
for i=1:length(recvis.vis)
  nblocked(i,:)=sum((recvis.vis(i).v==0)');
  [sn,ord]=sort(nblocked(i,:),'descend');
  if sn(1)>20 & min(sn(1:3))>3
    selcam(ord(1),i)=1;
  end
end
setfig('nblocked');
subplot(211);
plot(nblocked);
ylabel('Num pixels blocked');
xlabel('Sample number');
subplot(212);
plot(selcam');
ylabel('Camera to calibrate');
xlabel('Sample number');
cpos=recvis.layout.cpos;
ncameras=length(recvis.p.camera);
ncpos=nan(size(cpos));
setfig('locatecams')
clf;
for i=1:ncameras
  xp=cpos(i,:);
  fprintf('Calibrating camera %d at [%.3f,%.3f]\n', i,xp);

  subplot(2,2,i);
  hold on;
  plot(recvis.layout.active(1:end-1,1),recvis.layout.active(1:end-1,2),'m');
  plot(xp(1),xp(2),'r.');
  axis equal
  title(sprintf('Camera %d',i));
  others=1:ncameras;
  others=others(others~=i);
  fsel=find(selcam(i,:));
  lines=[];
  for j=1:length(fsel)
    fprintf('Sample %d ', fsel(j));
    v=recvis.vis(fsel(j)).v;
    offleds=find(sum(v(others,:)==0)>=2  & sum(v(others,:)==1)==0);   % Which LEDs are off

    fprintf('Off LEDS = %s ',shortlist(offleds));
    if length(offleds)<2
      fprintf('Skipping, only %d LEDs consistently blocked\n',length(offleds));
      continue;
    end
    if max(diff(offleds))>1
      fprintf('Skipping, off-LEDs not contiguous\n');
      continue;
    end
    if any(recvis.layout.ldir(offleds(1),:)~=recvis.layout.ldir(offleds(end),:))
      fprintf('Skipping, off-LEDs not collinear\n');
    end
    lp=mean(recvis.layout.lpos(offleds([1,end]),:));
    fprintf('Off-LEDs center = [%.3f,%.3f] ',lp');

    tgtleds=find(v(i,:)==0);   % Which LEDs are off
    fprintf('Tgt LEDS = %s ',shortlist(tgtleds));
    inter=sum(v(i,tgtleds(1):tgtleds(end))==1);
    if inter>0
      fprintf('Skipping, Tgt LEDs not contiguous (%d active intermediates)\n',inter);
      continue;
    end
    % Make sure next LED is actively on
    if tgtleds(1)==1 || tgtleds(end)==size(v,2) || v(i,tgtleds(1)-1)~=1 || v(i,tgtleds(end)+1)~=1
      fprintf('Skipping, adj tgt LED is not visible\n');
      continue;
    end
    endpos=recvis.layout.lpos(tgtleds([1,end]),:);
    plot([endpos(1,1),endpos(2,1)],[endpos(1,2),endpos(2,2)],'r');
    tlp=mean(endpos);
    fprintf('Tgt LEDs center = [%.3f,%.3f] ',tlp');

    % Compute equation of line between center of blocked LEDs from each direction
    plot(tlp(1),tlp(2),'o');
    plot(lp(1),lp(2),'x');
    dir=lp-tlp; dir=dir/norm(dir);
    extend=lp+dir*0.5;
    plot([tlp(1),extend(1)],[tlp(2),extend(2)]);
    k=-(tlp(2)-lp(2))/(tlp(1)-lp(1));
    b=sqrt(1/(1+k^2));
    a=k*b;
    c=-a*tlp(1)-b*tlp(2);
    dist=abs(a*xp(1)+b*xp(2)+c);
    fprintf('Err=%.3f\n',dist);
    lines(end+1,:)=[a b c];
  end
  % Set up as matrix equation  a * x + b * y = -c
  ab=lines(:,1:2);
  cc=-lines(:,3);
  ncpos(i,:)=ab\cc;
  fprintf('New position = [%.3f,%.3f], moved %.3f\n', ncpos(i,:), norm(cpos(i,:)-ncpos(i,:)));
  plot(ncpos(i,1),ncpos(i,2),'g.');
end
