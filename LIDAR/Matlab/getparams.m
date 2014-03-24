% Parameters for other modules
%
% Matching targets with tracks:
%  hiddenPenalty	
%
% Parameters for predicting positions of legs
%  initialPositionVar	Variance of initial position estimates
%  driftVar		Additional variance of position estimates per step when they are estimated 
% Parameters for classifying LIDAR scans
%  maxtgtsep		Max separation between points that are still the same target (unless there is a gap between)
%  maxbgsep		Max distance from background to be considered part of background
%  mintarget		Minimum unshadowed target size (otherwise is noise)
%  minrange		Minimum range, less than this becomes noise (dirt on sensor glass)
%  maxrange		Maximum range, outside this is ignored
function p=getparams()
p=struct(...
... % ******** Global
'fps',50,...
... % ******** Classification categories
... % ******** Background classification
'minbgsep',0.1,...
'tc',50*60,...
'minbgfreq',0.01,...
... % ******** Classifying LIDAR scans
'maxtgtsep',0.25,...   		% Max separation between points that are still the same target (unless there is a gap between)
'maxbgsep',0.1,...    		% Max distance from background to be considered part of background
'mintarget',0.1,...  		% Minimum unshadowed target size (otherwise is noise)
'minrange',0.1,...    		% Minimum range, less than this becomes noise (dirt on sensor glass)
'maxrange',5, ...      		% Maximum range, outside this is ignored
'maxclasssize',0.3, ...		% Maximum size of a single class in meters before it needs to be split
... % ******** Deleting tracks
'invisibleForTooLong',50,...	% Number of frames of invisible before deleting
'ageThreshold',20,...		% Age before considered reliable    
'minVisibility',0.6,...		% Minimum visibility to maintain new tracks (before ageThreshold reached)
... % ******** Likelihood analysis
'hiddenPenalty',3,...		% Extra loglike penalty when using a shadowed position for a leg
'newPersonPairMaxDist',0.5,...	% Maximum distance between pairs of new classes to attribute to same (new) person
... % ******** Predicting positions of legs
'initialPositionVar',0.1^2,...  % Variance of initial position estimates
'driftVar',0.1^2 ...  		% Additional variance of position estimates per step when they are estimated 
);

