% Turn front-end updates on/off
function feupdates(p,t1,t2)
if nargin<1 
  t1=0; % Turn off updates
  t2=0; % Turn off updates
elseif nargin<2
  t1=p.analysisparams.updatetc(1);
  t2=p.analysisparams.updatetc(2);
end
if t1==0 && t2==0
  fprintf('Turning off front end updates\n');
else
  fprintf('Setting update time constant to (%.0f,%.0f) seconds\n', t1, t2);
end
oscmsgout('FE','/vis/set/updatetc',{ t1, t2 });   
