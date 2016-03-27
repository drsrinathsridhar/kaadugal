% Use this script to generate 2D linear regression sample data with noise

clear all
close all

%-------- PARAMS ------------------
NumPoints = 2000;
Type = 'Train'; % 'Train/Test'

% NumPoints = 200;
% Type = 'Test'; % 'Train/Test'

xLimits = [1 NumPoints];
yLimits = [1 NumPoints];
OutputFile = sprintf('%sData2D_Regress_%d', Type, NumPoints)
OutDatFile = strcat(OutputFile, '.dat');
m1 = 0.5; % Slope 1
m2 = 0.5; % Slope 2
c = 0; % Intercept
Std = 5;
%-------- PARAMS ------------------

X = xLimits(1):1:xLimits(2);
Y = m1 * X + c;
% Add noise to Y
Y = Y + Std * randn(1, length(Y));
Z = m2 * Y;
% Add noise to Z
Z = Z + Std * randn(1, length(Z));

dlmwrite(OutDatFile, [X' Y' Z'], ' ')

% Plot
plot3(X, Y, Z, '.k')