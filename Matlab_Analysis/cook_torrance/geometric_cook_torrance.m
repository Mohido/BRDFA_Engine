function [G] = geometric_cook_torrance(V, L, N)
%GOEMETRIC_COOK_TORRANCE Gives the surface self masking/shadowing of its
%microfacets
%   Detailed explanation goes here

H = V+L;
H = H./norm(H);

masking = (2.* dot(N,H).* dot(N,V)) ./ dot(V,H);
shadowing = (2.* dot(N,H).* dot(N,L)) ./ dot(V,H);

%fprintf("Calculating Geometric Attenuation Function Using Cook-Torrance\n");
%fprintf("masking=%f , shadowing %f \n", masking, shadowing);
G = min([1 masking shadowing]);
end

