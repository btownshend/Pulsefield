% Convert a pmap from this aligner (scan + analyze) into JSON format for use by Calibrator

jsonin='/Users/bst/Dropbox/Pulsefield/src/modules/Calibrator/settings_proj.json';
jsonout='settings_proj_align.json';

fd=fopen(jsonin,'r');
if fd<0
  error('Unable to open %s',jsonin);
end
origjson=char(fread(fd))';
fclose(fd);
orig=jsondecode(origjson);
mappings=orig.calibration.mappings;
% Check current mappings
for i=1:length(mappings)
  m=mappings(i);
  m.unit1=str2double(m.unit1);
  m.unit2=str2double(m.unit2);
  fprintf('Mapping %d, units %d,%d\n',i,m.unit1,m.unit2);
  for j=1:length(m.pairs)
    p=m.pairs(j);
    p.locked=str2double(p.locked);
    p.pt1.x=str2double(p.pt1.x);
    p.pt1.y=str2double(p.pt1.y);
    p.pt2.x=str2double(p.pt2.x);
    p.pt2.y=str2double(p.pt2.y);
    if p.locked==1
      fprintf(' Pairs %d - (%.2f,%.2f) -> (%.2f,%.2f)\n',j, p.pt1.x,p.pt1.y,p.pt2.x,p.pt2.y);
      if m.unit1<length(pmap) && m.unit2<length(pmap)
        % Compare using pmap
        [cx,cy]=proj2cam(pmap{m.unit1+1},p.pt1.x/1920,p.pt1.y/1080);
        [px,py]=cam2proj(pmap{m.unit2+1},cx,cy);
        px=px*1920;py=py*1080;
        err=norm([px-p.pt2.x,py-p.pt2.y]);
        fprintf('   PMAP (%.2f,%.2f) -> (%.2f,%.2f)  err=%.1f pixels\n',p.pt1.x,p.pt1.y,px,py,err);
        [cx,cy]=proj2cam(pmap{m.unit2+1},p.pt2.x/1920,p.pt2.y/1080);
        [px,py]=cam2proj(pmap{m.unit1+1},cx,cy);
        px=px*1920;py=py*1080;
        err=norm([px-p.pt1.x,py-p.pt1.y]);
        fprintf('   PMAP (%.2f,%.2f) -> (%.2f,%.2f) err=%.1f pixels\n',px,py,p.pt2.x,p.pt2.y,err);
      end
    end
  end
end
