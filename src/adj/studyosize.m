
N = 1e7;

% Create random image sizes, desired areas to be filled, and crop parameters
osize = ceil(1000+1000*rand(2,N));
desired = ceil(osize.*(.1+.9*rand(2,N)));
crop = floor(900*rand(2,N));

% Calculate the needed size as per Adjuster::neededScaledOriginalSize
csize = osize - crop;
ff = desired ./ csize;
ff = min(ff);
needed = round(osize.*ff);

% Apply cropping as per mapCropSize
ss = needed./osize;
scsize = round(ss.*csize);

% Calculate whether we fit snugly. If we do, both numbers should be zero.
delta = desired - scsize;
min(min(delta))
max(min(delta))

% I checked that this works when using ROUND but not when using CEIL or 
% FLOOR. I have not tried whether using CEIL or FLOOR in both steps of 
% the scaling algorithm can cancel their bad effects out.