function [F] = schlick_fresnel(L, N, F0)
%SCHLICK_FRESNEL Summary of this function goes here
%   Detailed explanation goes here
    F = F0 + (1 - F0).*(1 - dot(N, L)).^ 5;
    
end

