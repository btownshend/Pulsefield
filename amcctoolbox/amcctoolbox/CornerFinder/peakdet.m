function maxtab=peakdet(v)
% PEAKDET Detect peaks in a vector
% 
% [MAXTAB, MINTAB] = PEAKDET(V, DELTA) finds the local
% maxima and minima ("peaks") in the vector V.
% MAXTAB and MINTAB consists of two columns. Column 1
% contains indices in V, and column 2 the found values.
% 
% With [MAXTAB, MINTAB] = PEAKDET(V, DELTA, X) the indices
% in MAXTAB and MINTAB are replaced with the corresponding
% X-values.
% 
% A point is considered a maximum peak if it has the maximal
% value, and was preceded (to the left) by a value lower by
% DELTA.
% 
% Eli Billauer, 3.4.05 (Explicitly not copyrighted).
% This function is released to the public domain; Any use is allowed.
% 
% Modified by Abdallah Kassir 13/12/2009


x=1:length(v);
[v,x]=cadj(v,x);
delta=(max(v)-min(v))/4;

maxtab = [];

if delta==0
    return;
end

mn = Inf; mx = -Inf;
mxpos = NaN;

lookformax = 0;  %modified

for i=1:length(v)
  this = v(i);
  if this > mx, mx = this; mxpos = x(i); end
  if this < mn, mn = this; end
  
  if lookformax
    if this < mx-delta
      maxtab = [maxtab ; mxpos mx];
      mn = this;
      lookformax = 0;
    end  
  else
    if this > mn+delta
      mx = this; mxpos = x(i);
      lookformax = 1;
    end
  end
end
