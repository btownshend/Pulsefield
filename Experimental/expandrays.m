% Expand rays to avoid missing small targets, misalignments, or moving targets
% For each blocked ray, expand with indeterminate rays
% Use p.analysisparams.expandrays additional rays on each side
function [v,expanded]=expandrays(p,v)
expanded=false(size(v));
for c=1:p.analysisparams.expandrays
  expanded(:,c+1:end)=expanded(:,c+1:end)|(v(:,1:end-c)==0);
  expanded(:,1:end-c)=expanded(:,1:end-c)|(v(:,c+1:end)==0);
end
% Don't overwrite blocked
expanded(v==0)=false;
v(expanded)=nan;   % Mark any expanded ones as indeterminate

