% List recordings
files=dir([pfroot(),'/Recordings/*.mat']);
for i=1:length(files)
  f=files(i);
  zz=load([pfroot(),'/Recordings/',f.name],'note');
  if ~isfield(zz,'note')
    system(sprintf('mv %s/Recordings/%s %s/Recordings/BAD',pfroot(),f.name,pfroot()));
    fprintf('Moved %s to BAD\n', f.name);
  end
  if ~isfield(zz,'note')
    zz.note='(empty)';
  end
  if ~isfield(zz,'vis')
    zz.vis=[];
  end
  fprintf('%s %s %4.1fMB %s\n', f.name, f.date, round(f.bytes/1024/1024), zz.note);
end