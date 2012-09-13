% Start logging in a dated diary file
function startdiary(component)
if nargin<1
  % Stop current diary
  fprintf('Diary stopped at %s\n', datestr(now));
  diary off
  return;
end
dir=sprintf('/Users/bst/Dropbox/PeopleSensor/LOGS/%s', component);
if ~exist(dir,'dir')
  [success,msg]=mkdir(dir);
  if ~success
    fprintf('startdiary: Unable to make directory %s: %s\n', dir, msg);
    return;
  end
end
fname=sprintf('%s/%s.diary', dir, datestr(now,30));
diary off
diary(fname)
fprintf('Diary for %s started at %s\n', component, datestr(now));

