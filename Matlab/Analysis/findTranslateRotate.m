% Find the best (minimum RMSE) rotation+translation that rigidly transforms points from src to dst
% The resulting matrix, M, gives  d=M*src'
% src, dst - each row is a data point, with 2 columns (not homogenous)
% m - result (3x3 homogenous that minimizes ||[dst:1]'-M*[src:1]'||)
%   - returns an empty matrix if unable to solve
% flipped - true if a flipped axis gave a better fit (but return solution without a flip)
% Based on http://nghiaho.com/?page_id=671, from  ‘A Method for Registration of 3-D Shapes’, by Besl and McKay, 1992.
function [m,flipped]=findTranslateRotate(src,dst)
  assert(size(src,2)==2);
  assert(size(dst,2)==2);
  assert(size(src,1)==size(dst,1));
  m=[];flipped=false;
  if size(src,1)<2
    fprintf('findTranslateRotate: Not enough data points (%d<2)\n',size(src,1));
    return
  end
  % Find centroids of data
  dcentroid=mean(dst)
  scentroid=mean(src)
  % Compute covariance matrix
  H=zeros(2,2);
  for i=1:size(src,1)
    H=H+(src(i,:)-scentroid)'*(dst(i,:)-dcentroid);
  end
  H
  % Decompose covariance
  [U,S,V]=svd(H)
  % Compute rotation matrix
  R=V*U'
  if det(R)<0
    % Not a rotation matrix
    % Occurs usually when points are near colinear, so flipping an axis gives a better fit
    % Flip is back to the rotate-only sense
    fprintf('Flipping: det(R)=%f, S=[%f %f]\n',det(R),diag(S));
    V(:,2)=-V(:,2);
    R=V*U';
    flipped=true;
  else
    flipped=false;
  end
  % Compute translation
  T=-R*scentroid'+dcentroid';
  % Combine into a homongenous matrix
  m=R;
  m(1:2,3)=T;
  m(3,3)=1;
end


