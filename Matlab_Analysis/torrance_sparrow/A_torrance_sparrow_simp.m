function [A] = A_torrance_sparrow_simp(v_i, v_r, n)
%A_TORRANCE_SPARROW_SIMP Summary of this function goes here
%   Detailed explanation goes here
x = dot(v_r,n); % incident direction projected to normal (cos(theta))
y = dot(v_r, v_i); % difference between incident direction to reflection direction

nom = 2*x*x + y - 1;
den = 2*y*x*x - y + 1;
A = -(nom/den);
end

