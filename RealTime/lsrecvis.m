% List recordings
files=dir('Recordings/*.mat');
for i=1:length(files)
  f=files(i);
  zz=load(['Recordings/',f.name],'note','vis');
  if ~isfield(zz,'note') || ~isfield(zz,'vis')
    system(sprintf('mv Recordings/%s Recordings/BAD',f.name));
    fprintf('Moved %s to BAD\n', f.name);
  end
  if ~isfield(zz,'note')
    zz.note='(empty)';
  end
  if ~isfield(zz,'vis')
    zz.vis=[];
  end
  fprintf('%s %s %5d frames %s\n', f.name, f.date, length(zz.vis), zz.note);
end