% Use this script to generate 2D structured output data for training and
% testing purposes

clear all
close all

% All parameters go here
xLimits = [100 200];
yLimits = [100 200];

xLevels = 4;
yLevels = 4;

NumPoints = 200;
SamplingSDFactor = 1/5; % This decides how tight the Gaussian sampling should be. Smaller is tighter.
Type = 'Test'; % 'Train'
OutputFile = sprintf('%sData2D_Struct_%dLevels_%fSampling_%dPts', Type, xLevels, SamplingSDFactor, NumPoints)
OutDatFile = strcat(OutputFile, '.dat');
% All parameters END here

xBinSize = abs(diff(xLimits)) / xLevels;
yBinSize = abs(diff(xLimits)) / yLevels;
nGridPts = xLevels * yLevels;
PtColors = hsv(nGridPts);

fig = figure(1);
xDir = linspace(xLimits(1)+xBinSize/2, xLimits(2)+xBinSize/2, xLevels+1);
xDir(end) = [];
yDir = linspace(yLimits(1)+yBinSize/2, yLimits(2)+yBinSize/2, yLevels+1);
yDir(end) = [];
xIdx = 0:xLevels-1;
yIdx = 0:yLevels-1;
[X, Y] = ndgrid(xDir, yDir);
[XIdx, YIdx] = ndgrid(xIdx, yIdx);
BinGrid = cat(3, X, Y);
BinAddresses = cat(3, XIdx, YIdx);
BinGrid = reshape(BinGrid, nGridPts, 2);
BinAddresses = reshape(BinAddresses, nGridPts, 2);
plot(BinGrid(:, 1), BinGrid(:, 2), 'or', 'MarkerSize', 15), hold on
for i = 1:nGridPts
    plot(BinGrid(i, 1), BinGrid(i, 2), '.', 'MarkerSize', 20, 'color', PtColors(i, :)), hold on
end

% Generate random points
PerturbPt = [];
PerturbPtColIdx = [];
OutputMat = [];
for i = 1:NumPoints
    % Pick random bin address
    PtIdx = randi(nGridPts);
    BinAdd = BinGrid(PtIdx, :); % Uniform distribution
    % Now create a perturbed point that is around the bin address
    % Perturb = [rand*xBinSize - xBinSize/2, rand*yBinSize - yBinSize/2]; % Uniform
    Perturb = [normrnd(0, xBinSize*SamplingSDFactor), normrnd(0, yBinSize*SamplingSDFactor)]; % Normal
    PerturbPt = [PerturbPt; BinAdd + Perturb];
    PerturbPtColIdx = [PerturbPtColIdx; PtIdx];
    
    OutputMat = [OutputMat; BinAdd + Perturb, BinAddresses(PtIdx, :)];
    
    plot(PerturbPt(i, 1), PerturbPt(i, 2), '.', 'MarkerSize', 10, 'color', PtColors(PerturbPtColIdx(i), :)), hold on
end

% Write output
dlmwrite(OutDatFile, OutputMat, ' ');

xlim([xLimits(1), xLimits(2)+xBinSize/2])
ylim([yLimits(1), yLimits(2)+yBinSize/2])
set(gca,'xtick', [xLimits(1):xBinSize:xLimits(2)])
set(gca,'ytick', [yLimits(1):yBinSize:yLimits(2)])
grid on

saveas(fig, strcat(OutputFile, '.png'), 'png');
