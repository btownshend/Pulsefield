function checkCC(CC,~)
%CHECKCC validates bwconncomp structure

%   Copyright 2008-2011 The MathWorks, Inc.
%   $Revision: 1.1.6.5 $  $Date: 2011/07/19 23:54:59 $

if ~isstruct(CC)
    error(message('images:checkCC:expectedStruct'));
end

tf = isfield(CC, {'Connectivity','ImageSize','NumObjects','PixelIdxList'});
if ~all(tf)
    error(message('images:checkCC:missingField'));
end
