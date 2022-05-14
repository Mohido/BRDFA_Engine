function [] = draw_axis()
%DRAW_AXIS Summary of this function goes here
%   Detailed explanation goes here
vectarrow([0 0 0], [0 0 1]);
hold on
axis equal
vectarrow([0 0 0], [0 1 0]);
hold on
axis equal
vectarrow([0 0 0], [0 -1 0]);
hold on
axis equal
vectarrow([0 0 0], [1 0 0]);
hold on
axis equal
vectarrow([0 0 0], [-1 0 0]);
hold on
axis equal
end

