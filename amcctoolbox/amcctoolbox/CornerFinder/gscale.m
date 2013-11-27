function g=gscale(f,method,low,high)
% GSCALE adjusts the scale of an image.
% 
% GSCALE returns the input image with the scale adjusted according to the
% input specifications.
%
% USAGE:
%     g=gscale(f,'minmax'); adjusts the image to the default min and max of
%     class of the input image
%     
%     g=gscale(f,'full8'); adjusts the image to a 8bit scale
% 
%     g=gscale(f,'full16'); adjsuts the image to a 16bit scale
% 
% INPUTS:
%     f: grayscale image (any class)
%     method: scaling method
% 
% OUTPUTS:
%     g: adjusted image

% input error checking
if ~exist('f','var') || isempty(f)
    error('Please Input Image');
end
if ~exist('method','var') || isempty(method)
    error('Please Specify Method');
end
if ~exist('low','var') || isempty(low)
    low=0;
end
if ~exist('high','var') || isempty(high)
    high=1;
end
if numel(low)>1 || numel(high)>1
    error('Low and High should be scalars');
end
if low>1 || low<0 || high>1 || high<0
    error('Low and High should be between 0 and 1');
end

g=double(f);
low=double(low);
high=double(high);
gmin=min(g(:));
gmax=max(g(:));

%Adjust to [0,1]
g=(g-gmin)./(gmax-gmin);


switch method
    case 'full8'
        g=im2uint8(g);
    case 'full16'
        g=im2uint16(g);
    case 'minmax'
        g=g.*(high-low)+low;
        switch class(f)
            case 'uint8'
                g=im2uint8(g);
            case 'uint16'
                g=im2uint16(g);
            case 'double'
                g=im2double(g);
            otherwise
                error('Unsupported Format, Supported Formats: unit8, uint16, double');
        end
    otherwise
        error('Invalid Method');
end